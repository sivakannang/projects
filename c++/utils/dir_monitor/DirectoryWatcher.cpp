// DirectoryWatcher.cpp
#include "DirectoryWatcher.hpp"

#include <sys/inotify.h>
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

uint32_t DirectoryWatcher::defaultMask() {
	return IN_CREATE | IN_DELETE | IN_MODIFY |
		IN_MOVED_FROM | IN_MOVED_TO |
		IN_CLOSE_WRITE | IN_ATTRIB;
}

DirectoryWatcher::DirectoryWatcher(const Path& root, uint32_t mask)
	: m_root(root),
	m_fd(-1),
	m_mask(mask),
	m_running(false)
{
	if (!fs::exists(m_root) || !fs::is_directory(m_root)) {
		throw std::runtime_error("Root is not directory: " + m_root.string());
	}

	m_fd = inotify_init1(IN_NONBLOCK);
	if (m_fd == -1) {
		throw std::runtime_error(
				std::string("inotify_init1 failed: ") + std::strerror(errno)
				);
	}

	addWatchRecursive(m_root);
}

DirectoryWatcher::~DirectoryWatcher() {
	stop();
	if (m_fd != -1) {
		for (auto &kv : m_wdToPath)
			inotify_rm_watch(m_fd, kv.first);
		close(m_fd);
		m_fd = -1;
	}
}

void DirectoryWatcher::stop() {
	m_running = false;
}

bool DirectoryWatcher::shouldIgnoreFile(const Path& file) const {
	std::string name = file.filename().string();
	if (name.empty()) return true;

	if (name[0] == '.') return true;
	if (name.ends_with(".swp") ||
			name.ends_with(".swx") ||
			name.ends_with(".tmp") ||
			name.ends_with("~"))
		return true;

	return false;
}

bool DirectoryWatcher::shouldWatchDir(const Path& dir) const {
	std::string name = dir.filename().string();
	if (name.empty()) return false;

	if (name == "out") return false;
	if (name[0] == '.') return false;

	return true;
}

void DirectoryWatcher::addWatch(const Path& dir) {
	if (!shouldWatchDir(dir)) return;

	int wd = inotify_add_watch(m_fd, dir.c_str(), m_mask);
	if (wd == -1) {
		std::cerr << "Failed watch on " << dir << ": "
			<< std::strerror(errno) << "\n";
		return;
	}
	m_wdToPath[wd] = dir;
}

void DirectoryWatcher::addWatchRecursive(const Path& root) {
	addWatch(root);

	for (auto it = fs::recursive_directory_iterator(root);
			it != fs::recursive_directory_iterator(); ++it) {
		if (!it->is_directory()) continue;

		Path d = it->path();
		std::string name = d.filename().string();

		if (name == "out") {
			it.disable_recursion_pending();
			continue;
		}

		addWatch(d);
	}
}

void DirectoryWatcher::removeWatch(int wd) {
	auto it = m_wdToPath.find(wd);
	if (it != m_wdToPath.end()) {
		inotify_rm_watch(m_fd, wd);
		m_wdToPath.erase(it);
	}
}

int DirectoryWatcher::strength(FileEventType t) {
	switch (t) {
		case FileEventType::Move: return 5;
		case FileEventType::Create: return 4;
		case FileEventType::Delete: return 4;
		case FileEventType::CloseWrite: return 3;
		case FileEventType::Modify: return 2;
		default: return 1;
	}
}

FileEventType DirectoryWatcher::mergeStrength(FileEventType a, FileEventType b) {
	return (strength(a) >= strength(b)) ? a : b;
}

void DirectoryWatcher::emitMerged(const Event &e) {
	if (!onEvent) return;
	onEvent(e);
}

void DirectoryWatcher::emitDebounced(const Event& e) {
	std::string key = e.path.string();
	auto now = std::chrono::steady_clock::now();

	auto &entry = m_debounce[key];

	if (entry.strongest == FileEventType::Unknown) {
		entry.path = e.path;
		entry.oldPath = e.oldPath;
		entry.isDir = e.isDirectory;
		entry.strongest = e.type;
		entry.lastUpdate = now;
	} else {
		entry.strongest = mergeStrength(entry.strongest, e.type);
		entry.lastUpdate = now;
	}
}

void DirectoryWatcher::flushDebounce(bool force) {
	if (m_debounce.empty()) return;

	auto now = std::chrono::steady_clock::now();
	std::vector<std::string> toErase;

	for (auto &kv : m_debounce) {
		auto &entry = kv.second;

		if (force ||
				now - entry.lastUpdate >= debounceWindow) {
			Event e;
			e.type = entry.strongest;
			e.path = entry.path;
			e.oldPath = entry.oldPath;
			e.isDirectory = entry.isDir;
			e.rawMask = 0;

			emitMerged(e);
			toErase.push_back(kv.first);
		}
	}

	for (auto &k : toErase)
		m_debounce.erase(k);
}

void DirectoryWatcher::emitEvent(uint32_t mask,
		const Path& fullPath,
		bool isDir,
		uint32_t cookie)
{
	if (shouldIgnoreFile(fullPath) && !isDir) {
		return;
	}

	if (mask & IN_MOVED_FROM) {
		m_pendingRename[cookie] = {
			fullPath,
			isDir,
			std::chrono::steady_clock::now()
		};
		return;
	}

	if (mask & IN_MOVED_TO) {
		auto it = m_pendingRename.find(cookie);
		if (it != m_pendingRename.end()) {
			Event ev;
			ev.type = FileEventType::Move;
			ev.path = fullPath;
			ev.oldPath = it->second.oldPath;
			ev.isDirectory = isDir;
			ev.rawMask = mask;

			m_pendingRename.erase(it);
			emitDebounced(ev);
			return;
		}
	}

	FileEventType type = FileEventType::Unknown;

	if (mask & IN_CREATE) type = FileEventType::Create;
	else if (mask & IN_DELETE) type = FileEventType::Delete;
	else if (mask & IN_MODIFY) type = FileEventType::Modify;
	else if (mask & IN_CLOSE_WRITE) type = FileEventType::CloseWrite;
	else if (mask & IN_ATTRIB) type = FileEventType::Modify;

	if (type == FileEventType::Unknown) return;

	Event ev;
	ev.type = type;
	ev.path = fullPath;
	ev.isDirectory = isDir;
	ev.rawMask = mask;

	emitDebounced(ev);
}

void DirectoryWatcher::handleEvent(const struct inotify_event* ev) {
	auto it = m_wdToPath.find(ev->wd);
	if (it == m_wdToPath.end()) return;

	Path dir = it->second;
	Path full = dir;
	if (ev->len > 0 && ev->name[0] != '\0')
		full /= ev->name;

	bool isDir = ev->mask & IN_ISDIR;

	if ((ev->mask & IN_CREATE) && isDir) {
		if (shouldWatchDir(full))
			addWatch(full);
	}

	if (ev->mask & IN_DELETE_SELF)
		removeWatch(ev->wd);

	emitEvent(ev->mask, full, isDir, ev->cookie);
}

void DirectoryWatcher::run() {
	m_running = true;

	constexpr size_t BUF_LEN = 4096;
	std::vector<char> buf(BUF_LEN);

	while (m_running) {
		flushDebounce(false);

		ssize_t len = read(m_fd, buf.data(), buf.size());
		if (len < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}
			if (errno == EINTR)
				continue;

			std::cerr << "read() failed: " << std::strerror(errno) << "\n";
			break;
		}

		size_t off = 0;
		while (off < (size_t)len) {
			auto *ev = reinterpret_cast<struct inotify_event*>(buf.data() + off);
			handleEvent(ev);
			off += sizeof(struct inotify_event) + ev->len;
		}
	}

	flushDebounce(true);
}

