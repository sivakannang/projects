#pragma once
// Simple logging for startup, shutdown and errors.
//
// NOT for the per-order hot path: one call costs microseconds (clock read, text
// formatting, a write() system call). Per-order events go to shared-memory counters
// instead; an asynchronous binary logger comes in a later task.
//
//   LOG_INFO("pinned to CPU %d", cpu);
//   -> 14:03:07.512 INFO  [processor 4242] pinned to CPU 1

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <unistd.h>

namespace lab {

// Process name shown in every line; set once at startup
inline const char*& logName() {
    static const char* name = "?";
    return name;
}

__attribute__((format(printf, 2, 3)))
inline void logLine(const char* level, const char* fmt, ...) {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t secs = system_clock::to_time_t(now);
    int ms = int(duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000);
    std::tm tm{};
    localtime_r(&secs, &tm);

    char buf[1024];
    int n = std::snprintf(buf, sizeof buf, "%02d:%02d:%02d.%03d %-5s [%s %d] ", tm.tm_hour, tm.tm_min,
                          tm.tm_sec, ms, level, logName(), int(getpid()));
    va_list ap;
    va_start(ap, fmt);
    n += std::vsnprintf(buf + n, sizeof buf - n, fmt, ap);
    va_end(ap);
    if (n > int(sizeof buf) - 2) n = int(sizeof buf) - 2;  // message was truncated
    buf[n++] = '\n';

    // One write() per line, so lines from different processes never interleave mid-line
    if (::write(STDERR_FILENO, buf, size_t(n)) < 0) {}
}

}  // namespace lab

#define LOG_INFO(...) ::lab::logLine("INFO", __VA_ARGS__)
#define LOG_WARN(...) ::lab::logLine("WARN", __VA_ARGS__)
#define LOG_ERROR(...) ::lab::logLine("ERROR", __VA_ARGS__)
