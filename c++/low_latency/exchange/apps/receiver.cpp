// receiver: Receives orders from the feeder and passes them to the processor.
// Task 10: pass-through, stamps when it handled each message.

#include "pipeline.hpp"
#include "timing.hpp"

int main(int argc, char** argv) {
    lab::Args args(argc, argv, "Receives orders from the feeder and passes them to the processor");
    lab::StageOptions opt = lab::stageOptions(args);
    args.done();

    return lab::runStage("receiver", opt, lab::kFeederToReceiver, lab::kReceiverToProcessor, [](lab::Msg& m) {
        m.hdr.tsReceiver = uint32_t(lab::now() - m.hdr.tsFeeder);
    });
}
