#!/bin/sh
# Pipeline test: runs the whole loop through scripts/run.sh (which starts, watches and cleans up),
# and checks that every program exits cleanly and no ring is left behind.
#
#   tests/pipeline_test.sh [bin dir] [orders]

BIN=${1:-build/release/bin}
COUNT=${2:-200000}
OUT=$(mktemp -d)

"$(dirname "$0")/../scripts/run.sh" --bin="$BIN" --out="$OUT" --timeout=60 -- --count="$COUNT" >"$OUT/output" 2>&1
fail=$?

if [ $fail -ne 0 ]; then
    cat "$OUT"/latest/*.log "$OUT/output"
    echo "pipeline test FAILED"
else
    grep -E "^run: (feeder|receiver|processor|sender) " "$OUT/output"
    echo "pipeline test passed ($COUNT orders)"
fi
rm -rf "$OUT"
exit $fail
