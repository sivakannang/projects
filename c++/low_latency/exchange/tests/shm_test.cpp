// Shared memory test: two real processes see the same memory, and bad segments are rejected.
//
// The parent creates a segment and writes a pattern. A fork()ed child attaches by name (getting
// its own mapping, at its own address), checks the pattern and writes a reply; the parent checks
// the reply. Then the failure cases: missing name, name already taken, replace, never ready,
// bad magic, wrong version.

#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#include "args.hpp"
#include "shm.hpp"

using namespace lab;

static int failures = 0;

static void check(bool ok, const char* what, const std::string& detail = "") {
    std::printf("  %-48s %s%s%s\n", what, ok ? "ok" : "FAILED", detail.empty() ? "" : "  ", detail.c_str());
    if (!ok) ++failures;
}

static uint8_t pattern(size_t i) { return uint8_t(i * 31 + 7); }

// Child process: attach, verify the parent's pattern, write the reply. Exit code = result.
static int child(const std::string& name, size_t bytes) {
    std::string err;
    auto shm = SharedMemory::open(name, &err);
    if (!shm) {
        std::printf("  child: open failed: %s\n", err.c_str());
        return 1;
    }
    if (shm->payloadSize() != bytes) return 2;
    auto* p = static_cast<uint8_t*>(shm->payload());
    for (size_t i = 0; i < bytes / 2; ++i)
        if (p[i] != pattern(i)) return 3;
    for (size_t i = bytes / 2; i < bytes; ++i) p[i] = uint8_t(~pattern(i));
    std::printf("  child pid %d mapped the segment at %p\n", int(getpid()), shm->payload());
    std::fflush(stdout);  // _exit() skips flushing
    return 0;
}

int main(int argc, char** argv) {
    Args args(argc, argv, "Shared memory segment test (uses fork)");
    size_t bytes = args.get<size_t>("bytes", 1 << 20, "payload size");
    args.done();

    const std::string name = "/lab_shm_test";
    std::string err;
    SharedMemory::unlink(name);  // leftover from an aborted earlier run

    std::printf("two processes, %zu bytes:\n", bytes);
    {
        auto shm = SharedMemory::create(name, bytes, &err);
        check(shm != nullptr, "create", err);
        if (!shm) return 1;
        auto* p = static_cast<uint8_t*>(shm->payload());
        for (size_t i = 0; i < bytes / 2; ++i) p[i] = pattern(i);
        shm->markReady();
        std::printf("  parent pid %d mapped the segment at %p\n", int(getpid()), shm->payload());

        std::fflush(stdout);  // or the child would print the parent's buffered output again
        pid_t pid = fork();
        if (pid == 0) _exit(child(name, bytes));
        int status = 0;
        waitpid(pid, &status, 0);
        check(WIFEXITED(status) && WEXITSTATUS(status) == 0, "child attached and verified the pattern",
              "exit code " + std::to_string(WEXITSTATUS(status)));

        bool replyOk = true;
        for (size_t i = bytes / 2; i < bytes; ++i) replyOk &= p[i] == uint8_t(~pattern(i));
        check(replyOk, "parent sees the child's writes");
        check(shm->header()->creatorPid == uint32_t(getpid()), "header records the creator pid");

        auto again = SharedMemory::create(name, bytes, &err);
        check(again == nullptr, "create again without replace is refused", err);

        err.clear();
        auto replaced = SharedMemory::create(name, 4096, &err, true);
        check(replaced != nullptr, "create with replace succeeds", err);
        check(p[0] == pattern(0), "old mapping still valid after replace");
        if (replaced) {
            check(static_cast<uint8_t*>(replaced->payload())[0] == 0, "replaced segment starts zeroed");

            // Not marked ready yet: attachers must time out
            auto early = SharedMemory::open(name, &err, 50);
            check(early == nullptr, "open before markReady times out", err);
            replaced->markReady();

            replaced->header()->magic[0] = 'X';
            auto bad = SharedMemory::open(name, &err, 50);
            check(bad == nullptr, "bad magic is rejected", err);
            replaced->header()->magic[0] = kShmMagic[0];

            replaced->header()->version = kShmVersion + 1;
            bad = SharedMemory::open(name, &err, 50);
            check(bad == nullptr, "wrong layout version is rejected", err);
            replaced->header()->version = kShmVersion;

            err.clear();
            auto good = SharedMemory::open(name, &err, 50);
            check(good != nullptr && good->payloadSize() == 4096, "valid segment opens", err);
        }
        err.clear();
        check(SharedMemory::unlink(name, &err), "unlink", err);
    }

    auto missing = SharedMemory::open(name, &err, 50);
    check(missing == nullptr, "open after unlink fails", err);
    check(!SharedMemory::unlink(name, &err), "unlink twice fails", err);
    auto badName = SharedMemory::create("no_slash", 64, &err);
    check(badName == nullptr, "bad name is rejected", err);

    std::printf("\n%s\n", failures ? "FAILED" : "all shared memory checks passed");
    return failures ? 1 : 0;
}
