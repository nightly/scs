#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
project_dir=$(cd -- "$script_dir/.." && pwd)
binary="$project_dir/bin/Release/scs_paper"
output_dir=""
suite="all"
repetitions=5
timeout_seconds=600
memory_mb=2048
cpu=""
paper_dir=""

usage() {
    printf '%s\n' \
        "Usage: scripts/run_paper_experiments.sh [options]" \
        "  --binary PATH          scs_paper executable (default: bin/Release/scs_paper)" \
        "  --output-dir PATH      new or empty result directory" \
        "  --suite NAME           runner suite (default: all)" \
        "  --repetitions N        fresh-process repetitions per case (default: 5)" \
        "  --timeout-seconds N    wall and cooperative timeout per case (default: 600)" \
        "  --memory-mb N          virtual-memory ceiling per case (default: 2048)" \
        "  --cpu N                optionally pin every run to one logical CPU" \
        "  --paper-dir PATH       also regenerate paper data and tables"
}

while (($#)); do
    case "$1" in
        --binary) binary=$2; shift 2 ;;
        --output-dir) output_dir=$2; shift 2 ;;
        --suite) suite=$2; shift 2 ;;
        --repetitions) repetitions=$2; shift 2 ;;
        --timeout-seconds) timeout_seconds=$2; shift 2 ;;
        --memory-mb) memory_mb=$2; shift 2 ;;
        --cpu) cpu=$2; shift 2 ;;
        --paper-dir) paper_dir=$2; shift 2 ;;
        --help|-h) usage; exit 0 ;;
        *) printf 'Unknown argument: %s\n' "$1" >&2; usage >&2; exit 2 ;;
    esac
done

for value in "$repetitions" "$timeout_seconds" "$memory_mb"; do
    [[ $value =~ ^[1-9][0-9]*$ ]] || { printf 'Expected a positive integer, got %s\n' "$value" >&2; exit 2; }
done
[[ -x $binary ]] || { printf 'Paper runner is not executable: %s\n' "$binary" >&2; exit 2; }
[[ -x /usr/bin/time ]] || { printf 'GNU /usr/bin/time is required\n' >&2; exit 2; }
command -v timeout >/dev/null || { printf 'GNU timeout is required\n' >&2; exit 2; }

if [[ -z $output_dir ]]; then
    output_dir="$project_dir/exports/paper-evaluation-$(date -u +%Y%m%dT%H%M%SZ)"
fi
if [[ -e $output_dir ]] && find "$output_dir" -mindepth 1 -print -quit | grep -q .; then
    printf 'Output directory is not empty: %s\n' "$output_dir" >&2
    exit 2
fi
mkdir -p "$output_dir/runs" "$output_dir/resources"
output_dir=$(cd -- "$output_dir" && pwd)

failures="$output_dir/failures.tsv"
printf 'case\trepetition\texit_status\n' > "$failures"
failed_runs=0

mapfile -t cases < <("$binary" --suite "$suite" --list)
((${#cases[@]})) || { printf 'Suite %s selected no cases\n' "$suite" >&2; exit 2; }

metadata="$output_dir/metadata.tsv"
printf 'key\tvalue\n' > "$metadata"
record() { printf '%s\t%s\n' "$1" "$2" >> "$metadata"; }
record generated_utc "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
record scs_commit "$(git -C "$project_dir" rev-parse HEAD)"
record scs_dirty_files "$(git -C "$project_dir" status --porcelain=v1 | wc -l)"
if [[ -d $project_dir/../scs-paper/.git ]]; then
    record paper_commit "$(git -C "$project_dir/../scs-paper" rev-parse HEAD)"
fi
record hostname "$(hostname)"
record kernel "$(uname -srmo)"
record cpu_model "$(lscpu | sed -n 's/^Model name:[[:space:]]*//p' | head -1)"
record logical_cpus "$(nproc)"
record memory_total_kib "$(awk '/MemTotal:/ {print $2}' /proc/meminfo)"
record compiler "$(c++ --version | head -1)"
record binary "$binary"
record suite "$suite"
record cases "${cases[*]}"
record repetitions "$repetitions"
record timeout_seconds "$timeout_seconds"
record memory_limit_mb "$memory_mb"
record cpu_pin "${cpu:-none}"
record load_average_start "$(cut -d ' ' -f 1-3 /proc/loadavg)"

timeout_ms=$((timeout_seconds * 1000))
memory_kb=$((memory_mb * 1024))
for case_name in "${cases[@]}"; do
    for ((repetition = 1; repetition <= repetitions; ++repetition)); do
        repeat_name=$(printf 'repeat-%02d' "$repetition")
        run_dir="$output_dir/runs/$case_name/$repeat_name"
        resource_file="$output_dir/resources/$case_name-$repeat_name.tsv"
        mkdir -p "$(dirname -- "$run_dir")"
        command=("$binary" --case "$case_name" --timeout-ms "$timeout_ms" --output-dir "$run_dir")
        if [[ -n $cpu ]]; then
            command=(taskset --cpu-list "$cpu" "${command[@]}")
        fi
        printf '[%s] %s %d/%d\n' "$(date +%H:%M:%S)" "$case_name" "$repetition" "$repetitions"
        if (
            ulimit -v "$memory_kb"
            exec /usr/bin/time -f 'peak_rss_kb\t%M\nelapsed_s\t%e' -o "$resource_file" \
                timeout --signal=TERM --kill-after=30s "$timeout_seconds" "${command[@]}"
        ); then
            if [[ ! -s $run_dir/metrics.tsv ]]; then
                printf '%s\t%d\tmissing-metrics\n' "$case_name" "$repetition" >> "$failures"
                ((failed_runs += 1))
            fi
        else
            status=$?
            printf '%s\t%d\t%d\n' "$case_name" "$repetition" "$status" >> "$failures"
            ((failed_runs += 1))
            printf 'Case %s repetition %d failed with status %d; continuing.\n' \
                "$case_name" "$repetition" "$status" >&2
        fi
    done
done
record load_average_end "$(cut -d ' ' -f 1-3 /proc/loadavg)"

summary_command=(python3 "$script_dir/summarize_paper_experiments.py" "$output_dir")
if [[ -n $paper_dir && $failed_runs -eq 0 ]]; then
    summary_command+=(--paper-dir "$paper_dir")
fi
"${summary_command[@]}"
printf 'Results written to %s\n' "$output_dir"
if ((failed_runs)); then
    printf '%d run(s) failed; see %s. Paper tables were not published.\n' "$failed_runs" "$failures" >&2
    exit 1
fi
