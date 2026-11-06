#include "cpu.hpp"

#include <pthread.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <fstream>

#if defined(__linux__)
#include <sched.h>
#else
#include <sys/sysctl.h>
#endif

namespace lab {

int onlineCpus() {
    return int(sysconf(_SC_NPROCESSORS_ONLN));
}

#if defined(__linux__)

bool pinningSupported() {
    return true;
}

std::string pinThread(int cpu) {
    if (cpu < 0 || cpu >= CPU_SETSIZE) return "CPU " + std::to_string(cpu) + " is out of range";
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    int rc = pthread_setaffinity_np(pthread_self(), sizeof set, &set);
    if (rc != 0)
        return std::string(std::strerror(rc)) + " (this machine has " + std::to_string(onlineCpus()) +
               " CPUs: 0-" + std::to_string(onlineCpus() - 1) + ")";
    return {};
}

int currentCpu() {
    return sched_getcpu();
}

std::string allowedCpus() {
    cpu_set_t set;
    CPU_ZERO(&set);
    if (pthread_getaffinity_np(pthread_self(), sizeof set, &set) != 0) return "?";
    // Compress to ranges: {0,1,2,3,6} -> "0-3,6"
    std::string out;
    for (int c = 0; c < CPU_SETSIZE; ++c) {
        if (!CPU_ISSET(c, &set)) continue;
        int last = c;
        while (last + 1 < CPU_SETSIZE && CPU_ISSET(last + 1, &set)) ++last;
        if (!out.empty()) out += ',';
        out += std::to_string(c);
        if (last > c) out += '-' + std::to_string(last);
        c = last;
    }
    return out;
}

void setThreadName(const char* name) {
    char shortName[16];
    std::snprintf(shortName, sizeof shortName, "%s", name);
    pthread_setname_np(pthread_self(), shortName);
}

std::string cpuModel() {
    std::ifstream in("/proc/cpuinfo");
    std::string line;
    while (std::getline(in, line))
        if (line.rfind("model name", 0) == 0) return line.substr(line.find(':') + 2);
    return "?";
}

std::string cpuGovernor() {
    std::ifstream in("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    std::string g;
    std::getline(in, g);
    return g.empty() ? "?" : g;
}

#else  // macOS and others

bool pinningSupported() {
    return false;
}

std::string pinThread(int) {
    return "CPU pinning is not supported on this OS";
}

int currentCpu() {
    return -1;
}

std::string allowedCpus() {
    return "all";
}

void setThreadName(const char* name) {
    pthread_setname_np(name);
}

std::string cpuModel() {
    char buf[128] = {};
    size_t len = sizeof buf - 1;
    if (sysctlbyname("machdep.cpu.brand_string", buf, &len, nullptr, 0) != 0) return "?";
    return buf;
}

std::string cpuGovernor() {
    return "";
}

#endif

}  // namespace lab
