// processor: Validates and matches orders, produces execution reports.
// Task 10: no matching yet. Answers every request with a dummy ExecReport in place
// (Accepted / Modified / Cancelled), so the return path already carries the real format.

#include "pipeline.hpp"
#include "timing.hpp"

using namespace lab;

static void dummyReport(Msg& m) {
    m.hdr.tsProcessor = uint32_t(now() - m.hdr.tsFeeder);

    // Read what we need before overwriting the body: all request types share the union
    uint64_t clientOrderId = 0;
    uint32_t qty = 0;
    ExecStatus status = ExecStatus::Rejected;
    RejectReason reject = RejectReason::BadMsgType;
    switch (m.hdr.type) {
        case MsgType::NewOrder:
            clientOrderId = m.newOrder.clientOrderId;
            qty = m.newOrder.qty;
            status = ExecStatus::Accepted;
            reject = RejectReason::None;
            break;
        case MsgType::ModifyOrder:
            clientOrderId = m.modify.clientOrderId;
            qty = m.modify.newQty;
            status = ExecStatus::Modified;
            reject = RejectReason::None;
            break;
        case MsgType::CancelOrder:
            clientOrderId = m.cancel.clientOrderId;
            status = ExecStatus::Cancelled;
            reject = RejectReason::None;
            break;
        default:
            break;
    }

    m.exec = ExecReport{};
    m.exec.clientOrderId = clientOrderId;
    m.exec.leavesQty = qty;
    m.exec.status = status;
    m.exec.reject = reject;
    m.hdr.type = MsgType::ExecReport;
}

int main(int argc, char** argv) {
    Args args(argc, argv, "Validates and matches orders, produces execution reports");
    StageOptions opt = stageOptions(args);
    args.done();

    return runStage("processor", opt, kReceiverToProcessor, kProcessorToSender, dummyReport);
}
