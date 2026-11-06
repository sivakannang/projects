#pragma once
// POSIX shared memory segment: shm_open + ftruncate + mmap.
//
// Layout of every segment:
//
//   [ ShmHeader: magic | version | sizes | creator pid | ready flag ][ payload ... ]
//   0                                                                 64
//
// The creator fills in the header, builds the payload, then calls markReady(). open() checks the magic, the layout
// version and the sizes, and waits for `ready`, so a process never uses a half-built segment
// or one written by an incompatible build. A segment whose creator is dead (left over from a
// crash) is treated as not there yet, so a restarted creator can replace it.
//
// A segment lives in /dev/shm (RAM) until shm_unlink, even if every process using it crashes.
// Destroying a SharedMemory object only unmaps it; only unlink() deletes the name.
//
//   auto shm = lab::SharedMemory::create("/lab_feed", bytes, &err);   // owner
//   ...build the payload...;  shm->markReady();
//   auto shm = lab::SharedMemory::open("/lab_feed", &err);            // others
//   void* p = shm->payload();

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace lab {

inline constexpr char kShmMagic[8] = {'L', 'A', 'B', 'S', 'H', 'M', '0', '1'};
inline constexpr uint32_t kShmVersion = 1;  // bump when the header or any payload layout changes

struct ShmHeader {
    char magic[8];
    uint32_t version;
    uint32_t headerSize;   // payload starts here
    uint64_t totalSize;    // header + payload
    uint64_t payloadSize;
    uint32_t creatorPid;
    std::atomic<uint32_t> ready;  // 0 while the creator is building the segment, 1 when usable
    uint8_t pad[24];
};
static_assert(sizeof(ShmHeader) == 64, "payload starts at offset 64");
static_assert(std::atomic<uint32_t>::is_always_lock_free, "a lock-based atomic would not work across processes");

class SharedMemory {
public:
    // New segment with `payloadBytes` of zeroed payload, not ready yet. Fails if the name exists, unless
    // replace = true: then the old name is unlinked first (processes still attached to the
    // old segment keep it until they detach, but nobody new can find it).
    static std::unique_ptr<SharedMemory> create(const std::string& name, size_t payloadBytes, std::string* err,
                                                bool replace = false);

    // Creator: payload is built, attachers may use it now
    void markReady();

    // Attach to a segment, waiting up to timeoutMs for the creator to create it and mark it ready
    static std::unique_ptr<SharedMemory> open(const std::string& name, std::string* err, int timeoutMs = 2000);

    // Delete the name (the creator does this at shutdown). Mapped memory stays valid.
    static bool unlink(const std::string& name, std::string* err = nullptr);

    ~SharedMemory();  // munmap only
    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    void* payload() const { return static_cast<char*>(base_) + sizeof(ShmHeader); }
    size_t payloadSize() const { return header()->payloadSize; }
    ShmHeader* header() const { return static_cast<ShmHeader*>(base_); }
    const std::string& name() const { return name_; }

private:
    // One attempt; *retry = true when waiting could help
    static std::unique_ptr<SharedMemory> tryOpen(const std::string& name, std::string* err, bool* retry);

    SharedMemory(std::string name, void* base, size_t mapped) : name_(std::move(name)), base_(base), mapped_(mapped) {}

    std::string name_;
    void* base_;
    size_t mapped_;
};

}  // namespace lab
