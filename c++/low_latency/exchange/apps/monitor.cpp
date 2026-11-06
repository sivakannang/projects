// monitor: Attaches to shared memory and prints live per-stage stats.

#include "args.hpp"
#include "process.hpp"

int main(int argc, char** argv) {
    lab::Args args(argc, argv, "Attaches to shared memory and prints live per-stage stats");
    int cpu = args.get<int>("cpu", -1, "CPU to pin this process to (-1 = don't pin)");
    args.done();

    lab::startProcess("monitor", cpu);
    return 0;
}
