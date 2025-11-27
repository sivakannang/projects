// DirectoryWatcher.hpp
#pragma once
#include <filesystem>
#include <functional>
#include <string>
#include <atomic>
#include <cstdint>

namespace fs = std::filesystem;

enum class FileEventType {
	Create,
	Delete,
	Modify,
	Move,
	CloseWrite,
	Unknown
};

struct FileEvent {
	FileEventType type;
	fs::path      path;      // For MOVE: new path
	fs::path      oldPath;   // For MOVE: old path
	bool          isDirectory;
	uint32_t      rawMask;
};

class DirectoryWatcher {
	public:
		using Event      = FileEvent;
		using EventCB    = std::function<void(const Event&)>;
		using Path       = fs::path;

		DirectoryWatcher(const Path& root, uint32_t mask = defaultMask());
		~DirectoryWatcher();

		void run();
		void stop();
		void setCallback(EventCB cb) { onEvent = cb; }

		static uint32_t defaultMask();

	private:
		// hidden implementation (pimpl-like but static)
		void addWatch(const Path& dir);
		void addWatchRecursive(const Path& root);
		void removeWatch(int wd);

		bool shouldWatchDir(const Path& dir) const;
		bool shouldIgnoreFile(const Path& file) const;

		void handleEvent(const struct inotify_event* ev);
		void emitEvent(uint32_t mask, const Path& path, bool isDir, uint32_t cookie);

		// Vim-safe merging + rename pairing + debounce
		void emitMerged(const Event& e);
		void emitDebounced(const Event& e);
		void flushDebounce(bool force = false);

		// internal state
		Path m_root;
		int  m_fd;
		uint32_t m_mask;
		std::atomic<bool> m_running;

		std::unordered_map<int, Path> m_wdToPath;

		struct MovePending {
			fs::path oldPath;
			bool isDir;
			std::chrono::steady_clock::time_point time;
		};
		std::unordered_map<uint32_t, MovePending> m_pendingRename;

		struct DebounceEntry {
			FileEventType strongest = FileEventType::Unknown;
			fs::path path;
			fs::path oldPath;
			bool isDir = false;
			std::chrono::steady_clock::time_point lastUpdate;
		};
		std::unordered_map<std::string, DebounceEntry> m_debounce;

		EventCB onEvent;
		const std::chrono::milliseconds debounceWindow{180};

		int strength(FileEventType t);
		FileEventType mergeStrength(FileEventType a, FileEventType b);
};

