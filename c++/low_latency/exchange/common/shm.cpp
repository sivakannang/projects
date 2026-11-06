#include "shm.hpp"

#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <new>
#include <thread>

namespace lab {

namespace {

bool fail(std::string* err, const std::string& msg) {
    if (err) *err = msg;
    return false;
}

std::string sysError(const std::string& what, const std::string& name) {
    return what + " " + name + ": " + std::strerror(errno);
}

// Portable POSIX names: "/name", no other '/', at most 31 characters (macOS limit)
bool validName(const std::string& name, std::string* err) {
    if (name.size() < 2 || name[0] != '/' || name.find('/', 1) != std::string::npos)
        return fail(err, "bad shared memory name '" + name + "' (must look like /lab_feed)");
    if (name.size() > 31) return fail(err, "shared memory name '" + name + "' is longer than 31 characters");
    return true;
}

}  // namespace

std::unique_ptr<SharedMemory> SharedMemory::create(const std::string& name, size_t payloadBytes, std::string* err,
                                                   bool replace) {
    if (!validName(name, err)) return nullptr;

    int fd = shm_open(name.c_str(), O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0 && errno == EEXIST && replace) {
        shm_unlink(name.c_str());
        fd = shm_open(name.c_str(), O_CREAT | O_EXCL | O_RDWR, 0600);
    }
    if (fd < 0) {
        if (errno == EEXIST)
            fail(err, "shared memory " + name + " already exists (left over from an earlier run?); remove it or create with replace");
        else
            fail(err, sysError("shm_open", name));
        return nullptr;
    }

    size_t total = sizeof(ShmHeader) + payloadBytes;
    if (ftruncate(fd, off_t(total)) != 0) {  // new pages read as zero
        fail(err, sysError("ftruncate", name));
        close(fd);
        shm_unlink(name.c_str());
        return nullptr;
    }
    void* base = mmap(nullptr, total, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);  // the mapping keeps the segment alive
    if (base == MAP_FAILED) {
        fail(err, sysError("mmap", name));
        shm_unlink(name.c_str());
        return nullptr;
    }

    auto* h = new (base) ShmHeader{};
    std::memcpy(h->magic, kShmMagic, sizeof h->magic);
    h->version = kShmVersion;
    h->headerSize = sizeof(ShmHeader);
    h->totalSize = total;
    h->payloadSize = payloadBytes;
    h->creatorPid = uint32_t(getpid());
    // Not ready yet: the caller builds the payload (e.g. a ring) and then calls markReady()
    return std::unique_ptr<SharedMemory>(new SharedMemory(name, base, total));
}

void SharedMemory::markReady() {
    header()->ready.store(1, std::memory_order_release);  // everything written before is visible to open()
}

std::unique_ptr<SharedMemory> SharedMemory::open(const std::string& name, std::string* err, int timeoutMs) {
    if (!validName(name, err)) return nullptr;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);

    // Situations that sort themselves out if we wait: the creator hasn't started yet, sits between
    // shm_open and ftruncate, is still building the payload, or the name belongs to a dead
    // creator from an earlier run and a new one is about to replace it.
    while (true) {
        bool retry = false;
        if (auto shm = tryOpen(name, err, &retry)) return shm;
        if (!retry || std::chrono::steady_clock::now() >= deadline) return nullptr;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

std::unique_ptr<SharedMemory> SharedMemory::tryOpen(const std::string& name, std::string* err, bool* retry) {
    int fd = shm_open(name.c_str(), O_RDWR, 0);
    if (fd < 0) {
        *retry = errno == ENOENT;
        fail(err, *retry ? "shared memory " + name + " does not exist (is its creator running?)"
                         : sysError("shm_open", name));
        return nullptr;
    }
    struct stat st{};
    if (fstat(fd, &st) != 0) {
        fail(err, sysError("fstat", name));
        close(fd);
        return nullptr;
    }
    if (size_t(st.st_size) < sizeof(ShmHeader)) {
        *retry = true;
        fail(err, "shared memory " + name + " has no size yet (creator stuck?)");
        close(fd);
        return nullptr;
    }

    size_t mapped = size_t(st.st_size);  // macOS may round this up to a whole page
    void* base = mmap(nullptr, mapped, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (base == MAP_FAILED) {
        fail(err, sysError("mmap", name));
        return nullptr;
    }
    std::unique_ptr<SharedMemory> shm(new SharedMemory(name, base, mapped));
    const ShmHeader* h = shm->header();

    // Creator sets `ready` last, so check it before trusting anything else in the header
    if (h->ready.load(std::memory_order_acquire) == 0) {
        *retry = true;
        fail(err, "shared memory " + name + " never became ready (creator crashed while building it?)");
        return nullptr;
    }
    if (std::memcmp(h->magic, kShmMagic, sizeof h->magic) != 0) {
        fail(err, "shared memory " + name + " was not created by this lab (bad magic)");
        return nullptr;
    }
    if (h->version != kShmVersion) {
        fail(err, "shared memory " + name + " has layout version " + std::to_string(h->version) + ", this build expects " +
                      std::to_string(kShmVersion) + " (rebuild everything / recreate the segment)");
        return nullptr;
    }
    if (h->headerSize != sizeof(ShmHeader) || h->totalSize > mapped ||
        h->totalSize != h->headerSize + h->payloadSize) {
        fail(err, "shared memory " + name + " has inconsistent sizes in its header");
        return nullptr;
    }
    // Left over from a crashed run: nobody will ever read it. Wait for the new creator instead.
    if (::kill(pid_t(h->creatorPid), 0) != 0 && errno == ESRCH) {
        *retry = true;
        fail(err, "shared memory " + name + " is left over from a dead process (pid " + std::to_string(h->creatorPid) +
                      "); its creator is not running");
        return nullptr;
    }
    return shm;
}

bool SharedMemory::unlink(const std::string& name, std::string* err) {
    if (shm_unlink(name.c_str()) != 0) return fail(err, sysError("shm_unlink", name));
    return true;
}

SharedMemory::~SharedMemory() {
    munmap(base_, mapped_);
}

}  // namespace lab
