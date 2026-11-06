#pragma once
// CPU pinning and thread naming.
// Linux: hard pinning (the thread only ever runs on that CPU).
// macOS: no pinning API exists (only affinity hints), so pinThread() reports it as unsupported.

#include <string>

namespace lab {

int onlineCpus();

// Pin the calling thread to one CPU. Returns "" on success, otherwise the reason it failed.
std::string pinThread(int cpu);

// True if pinThread() can work on this OS
bool pinningSupported();

// CPU the calling thread is running on right now, -1 if the OS can't tell
int currentCpu();

// CPUs the calling thread may run on, e.g. "1" or "0-3" (read back from the kernel)
std::string allowedCpus();

// Thread name shown in htop, perf and gdb (Linux keeps the first 15 characters)
void setThreadName(const char* name);

// For benchmark reports: CPU model name, and the frequency governor ("" if the OS has none)
std::string cpuModel();
std::string cpuGovernor();

}  // namespace lab
