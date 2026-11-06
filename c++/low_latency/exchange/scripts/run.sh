#!/usr/bin/env bash
# Runs one pipeline experiment: checks the machine, starts receiver, processor, sender and feeder
# (pinned if asked), watches them, always cleans up, and saves everything in a run folder.
#
#   scripts/run.sh [options] [-- feeder options]
#
#   scripts/run.sh --cpus=0,1,2,3 -- --rate=500000 --duration=10 --progress
#   scripts/run.sh -- --count=2000000              unpinned, as fast as possible
#   scripts/run.sh --clean                         kill stray lab programs, remove leftover rings
#
# Options:
#   --cpus=F,R,P,S   CPUs for feeder, receiver, processor, sender (default: not pinned)
#   --capacity=N     ring capacity for every program (default: the programs' own default)
#   --bin=DIR        where the programs are (default: build/release/bin)
#   --out=DIR        parent of the run folders (default: runs/); a run goes in <out>/<date_time>/
#   --timeout=S      stop everything after S seconds (default: 0 = no limit)
#   --clean          only clean up, start nothing
#
# A run folder holds command.txt, env.txt (code version, machine, CPU layout, OS tuning) and one
# log per program. Exit code: 0 if every program exited cleanly, 1 otherwise, 2 for bad options.
#
# Written for bash 3.2 (macOS) as well as Linux: no associative arrays, no `wait -n`.

ROOT=$(cd "$(dirname "$0")/.." && pwd)
NAMES=(feeder receiver processor sender)
RINGS=(lab_f2r lab_r2p lab_p2s lab_s2f)

CPUS=""
CAPACITY=""
BIN="$ROOT/build/release/bin"
OUT="$ROOT/runs"
TIMEOUT=0
CLEAN=0
FEEDER_ARGS=()

say()  { echo "run: $*"; }
warn() { echo "run: WARNING: $*"; }
die()  { echo "run: $*" >&2; exit 2; }

while [ $# -gt 0 ]; do
    case "$1" in
        --cpus=*)     CPUS=${1#*=} ;;
        --capacity=*) CAPACITY=${1#*=} ;;
        --bin=*)      BIN=${1#*=} ;;
        --out=*)      OUT=${1#*=} ;;
        --timeout=*)  TIMEOUT=${1#*=} ;;
        --clean)      CLEAN=1 ;;
        --)           shift; FEEDER_ARGS=("$@"); break ;;
        -h|--help)    sed -n '2,22p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *)            die "unknown option $1 (feeder options go after --; see --help)" ;;
    esac
    shift
done
BIN=$(cd "$BIN" 2>/dev/null && pwd -P) || die "no such folder: $BIN (run make first)"
LINUX=0; [ "$(uname -s)" = Linux ] && LINUX=1

# ---- helpers ----------------------------------------------------------------------------------

# Pids of lab programs started from this bin folder. A process counts only if its executable
# is $BIN/<name>, so a real program called "sender" on a shared host is never touched.
lab_pids() {
    local name pid
    for name in "${NAMES[@]}"; do
        for pid in $(pgrep -x "$name"); do
            if [ $LINUX = 1 ]; then
                [ "$(readlink "/proc/$pid/exe" 2>/dev/null)" = "$BIN/$name" ] && echo "$pid"
            else   # macOS shows the path as typed, maybe relative: good enough on a dev Mac
                case "$(ps -o comm= -p "$pid" 2>/dev/null)" in "$BIN/$name"|*/bin/"$name") echo "$pid" ;; esac
            fi
        done
    done
}

# Pid that created a ring: ShmHeader.creatorPid is a uint32 at byte offset 32
ring_creator() { od -An -t u4 -j 32 -N 4 "/dev/shm/$1" 2>/dev/null | tr -d ' '; }

# Removes leftover rings. Refuses (returns 1) if one belongs to a running process.
clean_rings() {
    [ $LINUX = 1 ] || return 0   # macOS can't list POSIX shm; the programs replace leftovers anyway
    local ring pid busy=0
    for ring in "${RINGS[@]}"; do
        [ -e "/dev/shm/$ring" ] || continue
        pid=$(ring_creator "$ring")
        if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null; then
            echo "run: /dev/shm/$ring belongs to pid $pid, which is still running (another run? try --clean)" >&2
            busy=1
        else
            rm -f "/dev/shm/$ring" && say "removed leftover /dev/shm/$ring"
        fi
    done
    return $busy
}

cpu_dir() { echo "/sys/devices/system/cpu/cpu$1"; }
cpu_core() { echo "$(cat "$(cpu_dir "$1")/topology/physical_package_id" 2>/dev/null)/$(cat "$(cpu_dir "$1")/topology/core_id" 2>/dev/null)"; }
cpu_node() { local d; for d in "$(cpu_dir "$1")"/node*; do [ -e "$d" ] && { basename "$d" | sed 's/node//'; return; }; done; echo "?"; }

# Busy % of each CPU over one second, from /proc/stat: "cpu busy%" per line
cpu_busy() {
    local a b
    a=$(grep '^cpu[0-9]' /proc/stat); sleep 1; b=$(grep '^cpu[0-9]' /proc/stat)
    printf '%s\n--\n%s\n' "$a" "$b" | awk '
        $1 == "--" { second = 1; next }
        { idle = $5 + $6; total = 0; for (i = 2; i <= NF; i++) total += $i
          if (!second) { i0[$1] = idle; t0[$1] = total }
          else if (total > t0[$1]) printf "%s %.0f\n", substr($1, 4), 100 * (1 - (idle - i0[$1]) / (total - t0[$1])) }'
}

# ---- --clean ----------------------------------------------------------------------------------

if [ $CLEAN = 1 ]; then
    pids=$(lab_pids)
    if [ -n "$pids" ]; then
        say "stopping stray lab programs: $(echo $pids)"
        kill -TERM $pids 2>/dev/null; sleep 1; kill -KILL $pids 2>/dev/null
    fi
    clean_rings
    say "clean"
    exit 0
fi

# ---- checks before starting -------------------------------------------------------------------

for name in "${NAMES[@]}"; do
    [ -x "$BIN/$name" ] || die "$BIN/$name not found (run make first)"
done
pids=$(lab_pids)
[ -z "$pids" ] || die "lab programs are already running (pids $(echo $pids)); wait for them or use --clean"
clean_rings || exit 2

CPU_LIST=()
if [ -n "$CPUS" ]; then
    IFS=, read -r -a CPU_LIST <<< "$CPUS"
    [ ${#CPU_LIST[@]} = 4 ] || die "--cpus needs 4 CPUs (feeder,receiver,processor,sender), got '$CPUS'"
    for i in 0 1 2 3; do
        c=${CPU_LIST[$i]}
        case "$c" in ''|*[!0-9]*) die "--cpus: '$c' is not a CPU number" ;; esac
        for j in 0 1 2 3; do
            [ $j -lt $i ] && [ "${CPU_LIST[$j]}" = "$c" ] && die "--cpus: CPU $c is used twice"
        done
        [ $LINUX = 0 ] || [ -d "$(cpu_dir "$c")" ] || die "--cpus: this machine has no CPU $c"
    done
fi

if [ $LINUX = 1 ] && [ ${#CPU_LIST[@]} = 4 ]; then
    # Two programs on one physical core share it as hyper-threads: much slower (task 6)
    for i in 0 1 2 3; do for j in 0 1 2 3; do
        [ $j -gt $i ] || continue
        a=${CPU_LIST[$i]}; b=${CPU_LIST[$j]}
        [ "$(cpu_core "$a")" = "$(cpu_core "$b")" ] &&
            warn "${NAMES[$i]} (CPU $a) and ${NAMES[$j]} (CPU $b) share a physical core (hyper-threads)"
    done; done
    # Every ring hop between NUMA nodes / sockets costs extra
    nodes=""; for c in "${CPU_LIST[@]}"; do nodes="$nodes $(cpu_node "$c")"; done
    [ "$(echo $nodes | tr ' ' '\n' | sort -u | wc -l)" -gt 1 ] && warn "the CPUs are on different NUMA nodes:$nodes"
    # On a shared host something else may already be running there
    busy=$(cpu_busy)
    for i in 0 1 2 3; do
        c=${CPU_LIST[$i]}
        pct=$(echo "$busy" | awk -v c="$c" '$1 == c { print $2 }')
        [ -n "$pct" ] && [ "$pct" -gt 20 ] && warn "CPU $c (${NAMES[$i]}) was ${pct}% busy before the run started"
    done
fi

gov=$(cat /sys/devices/system/cpu/cpu${CPU_LIST[0]:-0}/cpufreq/scaling_governor 2>/dev/null)
[ -n "$gov" ] && [ "$gov" != performance ] && warn "CPU governor is '$gov', not 'performance': latency will be noisier"

# ---- run folder -------------------------------------------------------------------------------

mkdir -p "$OUT" || die "cannot create $OUT"
RUN="$OUT/$(date +%Y-%m-%d_%H%M%S)"
n=1; while [ -e "$RUN" ]; do RUN="$OUT/$(date +%Y-%m-%d_%H%M%S)_$n"; n=$((n + 1)); done
mkdir "$RUN" || die "cannot create $RUN"
ln -sfn "$(basename "$RUN")" "$OUT/latest" 2>/dev/null

# The parser consumed the arguments, so rebuild the command line for the record
{
    printf 'scripts/run.sh'
    [ -n "$CPUS" ] && printf ' --cpus=%s' "$CPUS"
    [ -n "$CAPACITY" ] && printf ' --capacity=%s' "$CAPACITY"
    [ "$TIMEOUT" != 0 ] && printf ' --timeout=%s' "$TIMEOUT"
    printf ' --bin=%s --' "$BIN"
    for a in ${FEEDER_ARGS[@]+"${FEEDER_ARGS[@]}"}; do printf ' %s' "$a"; done
    echo
} > "$RUN/command.txt"

{
    echo "date:        $(date '+%Y-%m-%d %H:%M:%S %Z')"
    echo "host:        $(hostname)"
    if command -v git >/dev/null && git -C "$ROOT" rev-parse HEAD >/dev/null 2>&1; then
        dirty=""; [ -n "$(git -C "$ROOT" status --porcelain -- . 2>/dev/null)" ] && dirty=" + uncommitted changes"
        echo "code:        $(git -C "$ROOT" rev-parse --short HEAD)$dirty"
    else
        echo "code:        no git"
    fi
    echo "build:       $(basename "$(dirname "$BIN")") ($BIN)"
    echo "os:          $(uname -srm)"
    if [ $LINUX = 1 ]; then
        echo "cpu model:   $(grep -m1 'model name' /proc/cpuinfo | sed 's/.*: //')"
        command -v lscpu >/dev/null &&
            lscpu | grep -E '^(CPU\(s\)|Thread|Core|Socket|NUMA)' | sed 's/^/lscpu:       /'
        echo "governor:    ${gov:-unknown}"
        echo "isolated:    $(cat /sys/devices/system/cpu/isolated 2>/dev/null)"
        echo "boot line:   $(cat /proc/cmdline 2>/dev/null)"
        command -v tuned-adm >/dev/null && echo "tuned:       $(tuned-adm active 2>/dev/null)"
        echo "perf_event_paranoid: $(cat /proc/sys/kernel/perf_event_paranoid 2>/dev/null)"
    else
        echo "cpu model:   $(sysctl -n machdep.cpu.brand_string 2>/dev/null)"
        echo "cpus:        $(sysctl -n hw.ncpu 2>/dev/null) (pinning not supported on macOS)"
    fi
    for i in 0 1 2 3; do
        c=${CPU_LIST[$i]:-}
        if [ -z "$c" ]; then
            echo "${NAMES[$i]}:$(printf '%*s' $((12 - ${#NAMES[$i]})) '')not pinned"
        elif [ $LINUX = 1 ]; then
            echo "${NAMES[$i]}:$(printf '%*s' $((12 - ${#NAMES[$i]})) '')CPU $c (socket/core $(cpu_core "$c"), node $(cpu_node "$c"))"
        else
            echo "${NAMES[$i]}:$(printf '%*s' $((12 - ${#NAMES[$i]})) '')CPU $c"
        fi
    done
} > "$RUN/env.txt"

# ---- start ------------------------------------------------------------------------------------

PIDS=(); RCS=(); STOPPING=0; STOP_AT=0; TAIL=""

stop_all() {   # $1 = reason
    [ $STOPPING = 1 ] && return
    STOPPING=1; STOP_AT=$SECONDS
    say "$1: stopping the programs"
    for i in 0 1 2 3; do [ -z "${RCS[$i]}" ] && kill -TERM "${PIDS[$i]}" 2>/dev/null; done
}
trap 'echo; stop_all "interrupted"' INT TERM

# Stages first, the feeder last (any order works, this just keeps the logs tidy)
for i in 1 2 3 0; do
    args=()
    [ -n "${CPU_LIST[$i]:-}" ] && args+=(--cpu="${CPU_LIST[$i]}")
    [ -n "$CAPACITY" ] && args+=(--capacity="$CAPACITY")
    [ $i = 0 ] && args+=(${FEEDER_ARGS[@]+"${FEEDER_ARGS[@]}"})
    "$BIN/${NAMES[$i]}" ${args[@]+"${args[@]}"} > "$RUN/${NAMES[$i]}.log" 2>&1 &
    PIDS[$i]=$!
    RCS[$i]=""
done
say "run folder $RUN"
say "started feeder ${PIDS[0]}, receiver ${PIDS[1]}, processor ${PIDS[2]}, sender ${PIDS[3]}"
tail -n +1 -f "$RUN/feeder.log" & TAIL=$!   # show the feeder live

# ---- watch ------------------------------------------------------------------------------------

START=$SECONDS; FEEDER_DONE_AT=0
while true; do
    left=0
    for i in 0 1 2 3; do
        [ -n "${RCS[$i]}" ] && continue
        if kill -0 "${PIDS[$i]}" 2>/dev/null; then
            left=$((left + 1))
        else
            wait "${PIDS[$i]}"; RCS[$i]=$?
            [ ${RCS[$i]} != 0 ] && stop_all "${NAMES[$i]} exited with code ${RCS[$i]}"
            [ $i = 0 ] && FEEDER_DONE_AT=$SECONDS
        fi
    done
    [ $left = 0 ] && break

    # After the feeder's EndOfStream round trip every stage has already exited; give them 3 s
    [ -n "${RCS[0]}" ] && [ $((SECONDS - FEEDER_DONE_AT)) -ge 3 ] && stop_all "feeder finished but a stage is still running"
    [ "$TIMEOUT" != 0 ] && [ $((SECONDS - START)) -ge "$TIMEOUT" ] && stop_all "timeout of ${TIMEOUT} s"
    if [ $STOPPING = 1 ] && [ $((SECONDS - STOP_AT)) -ge 3 ]; then
        for i in 0 1 2 3; do
            [ -z "${RCS[$i]}" ] && kill -KILL "${PIDS[$i]}" 2>/dev/null && say "killed ${NAMES[$i]} (no exit after SIGTERM)"
        done
    fi
    sleep 0.2
done
sleep 0.2; kill $TAIL 2>/dev/null; wait $TAIL 2>/dev/null
trap - INT TERM

# ---- summary ----------------------------------------------------------------------------------

fail=0
echo
for i in 0 1 2 3; do
    rc=${RCS[$i]}
    [ "$rc" != 0 ] && fail=1
    last=$(grep -E 'end of stream|stopped' "$RUN/${NAMES[$i]}.log" | tail -1 | sed 's/.*\] //')
    [ $i = 0 ] && last=$(tail -1 "$RUN/feeder.log" | sed 's/.*\] //')
    [ "$rc" -gt 128 ] && last="killed by signal $((rc - 128))"
    printf 'run: %-10s exit %-3s %s\n' "${NAMES[$i]}" "$rc" "$last"
done

# A clean run leaves no rings behind; after a kill, leftovers are expected and removed
if [ $LINUX = 1 ]; then
    left=""; for ring in "${RINGS[@]}"; do [ -e "/dev/shm/$ring" ] && left="$left $ring"; done
    if [ -n "$left" ]; then
        if [ $fail = 0 ]; then warn "rings left behind after a clean run:$left"; fail=1; fi
        clean_rings >/dev/null
    fi
fi

if [ $fail = 0 ]; then say "OK, logs in $RUN"; else say "FAILED, logs in $RUN"; fi
exit $fail
