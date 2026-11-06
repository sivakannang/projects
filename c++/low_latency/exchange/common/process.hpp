#pragma once
// Common startup for every process: set its name, pin it if asked, and log where it runs.

#include <cstdlib>

#include "cpu.hpp"
#include "log.hpp"

namespace lab {

// cpu < 0 means "don't pin". On Linux a failed pin is fatal (a benchmark on the wrong
// CPU is worse than no benchmark); on macOS pinning doesn't exist, so it only warns.
inline void startProcess(const char* name, int cpu) {
    logName() = name;
    setThreadName(name);

    if (cpu >= 0) {
        std::string err = pinThread(cpu);
        if (!err.empty()) {
            if (pinningSupported()) {
                LOG_ERROR("cannot pin to CPU %d: %s", cpu, err.c_str());
                std::exit(1);
            }
            LOG_WARN("--cpu=%d ignored: %s", cpu, err.c_str());
        }
    }
    LOG_INFO("started, allowed CPUs %s, running on CPU %d", allowedCpus().c_str(), currentCpu());
}

}  // namespace lab
