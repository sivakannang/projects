// sender: Takes execution reports from the processor and sends them back to the feeder.
// Task 10: pass-through, stamps when it handled each message.

#include "pipeline.hpp"
#include "timing.hpp"

int main(int argc, char** argv) {
    lab::Args args(argc, argv, "Sends execution reports from the processor back to the feeder");
    lab::StageOptions opt = lab::stageOptions(args);
    args.done();

    return lab::runStage("sender", opt, lab::kProcessorToSender, lab::kSenderToFeeder, [](lab::Msg& m) {
        m.hdr.tsSender = uint32_t(lab::now() - m.hdr.tsFeeder);
    });
}
