#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./bench.sh --preset PRESET [options]

Builds (optional) and runs the benchmarkInference target using a CMake preset.

Options:
  -p, --preset NAME         CMake configure preset (required, e.g. bench-release)
  -o, --out PATH            Output JSON path
                             (default: benchmarks/results/<preset>.json)
  -f, --filter REGEX        Google Benchmark filter
      --min-time SECONDS    Minimum benchmark time (default: 0.01)
      --format FORMAT       json|console (default: json)
      --no-build            Skip build step
  -h, --help                Show this help

Examples:
  ./bench.sh --preset bench-release
  ./bench.sh --preset bench-release --filter Inference --min-time 0.05
EOF
}

die() { echo "error: $*" >&2; exit 1; }

# ------------------------------------------------------------
# Locate repo root
# ------------------------------------------------------------

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR"

if command -v git >/dev/null 2>&1; then
  ROOT_DIR="$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel 2>/dev/null || echo "$SCRIPT_DIR")"
fi

# ------------------------------------------------------------
# Defaults
# ------------------------------------------------------------

PRESET=""
OUT_PATH=""
BENCH_FILTER=""
MIN_TIME="0.01"
OUT_FORMAT="json"
DO_BUILD="1"

# ------------------------------------------------------------
# Argument parsing
# ------------------------------------------------------------

while [[ $# -gt 0 ]]; do
  case "$1" in
    -p|--preset)
      [[ $# -ge 2 ]] || die "missing value for $1"
      PRESET="$2"; shift 2 ;;
    -o|--out)
      [[ $# -ge 2 ]] || die "missing value for $1"
      OUT_PATH="$2"; shift 2 ;;
    -f|--filter)
      [[ $# -ge 2 ]] || die "missing value for $1"
      BENCH_FILTER="$2"; shift 2 ;;
    --min-time)
      [[ $# -ge 2 ]] || die "missing value for $1"
      MIN_TIME="$2"; shift 2 ;;
    --format)
      [[ $# -ge 2 ]] || die "missing value for $1"
      OUT_FORMAT="$2"; shift 2 ;;
    --no-build)
      DO_BUILD="0"; shift ;;
    -h|--help)
      usage; exit 0 ;;
    *)
      die "unknown argument: $1 (use --help)" ;;
  esac
done

[[ -n "$PRESET" ]] || die "--preset is required"

command -v cmake >/dev/null 2>&1 || die "cmake not found"

# ------------------------------------------------------------
# Normalize min time
# ------------------------------------------------------------

if [[ "$MIN_TIME" =~ ^[0-9]+([.][0-9]+)?$ ]]; then
  MIN_TIME="${MIN_TIME}s"
fi

# ------------------------------------------------------------
# Machine identity (POSIX)
# ------------------------------------------------------------

HOST="$(hostname | tr '[:upper:]' '[:lower:]' | tr -cd 'a-z0-9_-')"
OS="$(uname -s)"
ARCH="$(uname -m)"
STAMP="$(date +%Y%m%d-%H%M%S)"

# ------------------------------------------------------------
# Paths
# ------------------------------------------------------------

BUILD_DIR="$ROOT_DIR/build/$PRESET"

if [[ -z "$OUT_PATH" ]]; then
  OUT_PATH="$ROOT_DIR/benchmarks/results/${PRESET}__${HOST}__${OS}-${ARCH}__${STAMP}.json"
fi

mkdir -p "$(dirname "$OUT_PATH")"

# ------------------------------------------------------------
# Configure & build
# ------------------------------------------------------------

echo "== LogosLab benchmark runner =="
echo "root:     $ROOT_DIR"
echo "preset:   $PRESET"
echo "build:    $BUILD_DIR"
echo "out:      $OUT_PATH"
echo "format:   $OUT_FORMAT"
echo "min-time: $MIN_TIME"
[[ -n "$BENCH_FILTER" ]] && echo "filter:   $BENCH_FILTER"

cmake --preset "$PRESET"

if [[ "$DO_BUILD" == "1" ]]; then
  cmake --build --preset bench
fi

BENCH_EXE="$BUILD_DIR/benchmarkInference"
[[ -x "$BENCH_EXE" ]] || die "benchmark executable not found: $BENCH_EXE"

# ------------------------------------------------------------
# System info
# ------------------------------------------------------------

SYSINFO_PATH="${OUT_PATH%.json}.system.txt"
{
  echo "timestamp: $(date -Iseconds)"
  echo "preset: $PRESET"
  echo "build_dir: $BUILD_DIR"
  if command -v git >/dev/null 2>&1; then
    echo "git_sha: $(git -C "$ROOT_DIR" rev-parse HEAD 2>/dev/null || echo unknown)"
    echo "git_dirty: $(git -C "$ROOT_DIR" diff --quiet && echo no || echo yes)"
  fi
  echo "cmake: $(cmake --version | head -n 1)"
  echo "uname: $(uname -a)"
} > "$SYSINFO_PATH"

# ------------------------------------------------------------
# Benchmark args
# ------------------------------------------------------------

ARGS=( "--benchmark_min_time=$MIN_TIME" )

if [[ -n "$BENCH_FILTER" ]]; then
  ARGS+=( "--benchmark_filter=$BENCH_FILTER" )
fi

case "$OUT_FORMAT" in
  json)
    ARGS+=( "--benchmark_out=$OUT_PATH" "--benchmark_out_format=json" )
    ;;
  console)
    CONSOLE_PATH="${OUT_PATH%.json}.txt"
    "$BENCH_EXE" "${ARGS[@]}" --benchmark_format=console | tee "$CONSOLE_PATH"
    exit 0
    ;;
  *)
    die "unsupported format: $OUT_FORMAT"
    ;;
esac

"$BENCH_EXE" "${ARGS[@]}"

echo "wrote benchmark json: $OUT_PATH"
echo "wrote system info:    $SYSINFO_PATH"
