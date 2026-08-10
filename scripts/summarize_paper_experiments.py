#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import math
import statistics
from collections import defaultdict
from pathlib import Path


TIMINGS = (
    "synthesis_ms",
    "arena_ms",
    "qualitative_ms",
    "budget_ms",
    "extraction_ms",
    "validation_ms",
    "peak_rss_kb",
    "elapsed_s",
)

CASE_ORDER = (
    "finite-explicit",
    "finite-explicit-3",
    "finite-explicit-4",
    "faithful-breadth-first",
    "faithful-lower-cost-first",
    "faithful-greedy",
)

LABELS = {
    "finite-explicit": "Explicit (2 IDs)",
    "finite-explicit-3": "Explicit (3 IDs)",
    "finite-explicit-4": "Explicit (4 IDs)",
    "faithful-breadth-first": "Faithful / breadth-first",
    "faithful-lower-cost-first": "Faithful / lower-cost-first",
    "faithful-greedy": "Faithful / greedy",
}


def quantile(values: list[float], probability: float) -> float:
    ordered = sorted(values)
    if len(ordered) == 1:
        return ordered[0]
    position = (len(ordered) - 1) * probability
    lower = math.floor(position)
    upper = math.ceil(position)
    if lower == upper:
        return ordered[lower]
    return ordered[lower] + (ordered[upper] - ordered[lower]) * (position - lower)


def resource_values(path: Path) -> dict[str, str]:
    rows: dict[str, str] = {}
    with path.open(encoding="utf-8") as stream:
        for line in stream:
            key, value = line.rstrip("\n").split("\t", 1)
            rows[key] = value
    return rows


def read_runs(root: Path) -> tuple[list[str], list[dict[str, str]]]:
    rows: list[dict[str, str]] = []
    metric_header: list[str] | None = None
    for metrics_path in sorted((root / "runs").glob("*/repeat-*/metrics.tsv")):
        if metrics_path.stat().st_size == 0:
            continue
        with metrics_path.open(newline="", encoding="utf-8") as stream:
            reader = csv.DictReader(stream, delimiter="\t")
            current = list(reader)
            if len(current) != 1:
                raise RuntimeError(f"expected one metrics row in {metrics_path}, found {len(current)}")
            if metric_header is None:
                metric_header = list(reader.fieldnames or [])
            elif metric_header != list(reader.fieldnames or []):
                raise RuntimeError(f"metrics header differs in {metrics_path}")
        case_name = metrics_path.parent.parent.name
        repetition = metrics_path.parent.name.removeprefix("repeat-")
        resource_path = root / "resources" / f"{case_name}-repeat-{repetition}.tsv"
        if not resource_path.exists():
            raise RuntimeError(f"missing resource measurement {resource_path}")
        row = current[0]
        row.update(resource_values(resource_path))
        row["repetition"] = str(int(repetition))
        rows.append(row)
    if not rows or metric_header is None:
        raise RuntimeError(f"no experiment rows found below {root / 'runs'}")
    return metric_header, rows


def summarize(metric_header: list[str], rows: list[dict[str, str]]) -> list[dict[str, str]]:
    grouped: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        grouped[row["case"]].append(row)
    summaries: list[dict[str, str]] = []
    for case_name in CASE_ORDER:
        group = grouped.get(case_name)
        if not group:
            continue
        result: dict[str, str] = {"case": case_name, "repetitions": str(len(group))}
        for field in metric_header:
            if field == "case" or field in TIMINGS:
                continue
            values = {row[field] for row in group}
            if len(values) != 1:
                raise RuntimeError(f"non-timing metric {field} changed across {case_name}: {sorted(values)}")
            result[field] = values.pop()
        for field in TIMINGS:
            values = [float(row[field]) for row in group]
            median = statistics.median(values)
            iqr = quantile(values, 0.75) - quantile(values, 0.25)
            result[f"{field}_median"] = f"{median:.6f}"
            result[f"{field}_iqr"] = f"{iqr:.6f}"
        summaries.append(result)
    unknown = sorted(set(grouped).difference(CASE_ORDER))
    if unknown:
        raise RuntimeError(f"summary order is undefined for cases: {unknown}")
    return summaries


def write_tsv(path: Path, rows: list[dict[str, str]], fieldnames: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=fieldnames, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def latex_time(row: dict[str, str], field: str = "arena_ms") -> str:
    return f"{float(row[field + '_median']):.1f} [{float(row[field + '_iqr']):.1f}]"


def latex_memory(row: dict[str, str]) -> str:
    median = float(row["peak_rss_kb_median"]) / 1024.0
    iqr = float(row["peak_rss_kb_iqr"]) / 1024.0
    return f"{median:.1f} [{iqr:.1f}]"


def write_latex_table(path: Path, caption: str, label: str, columns: str,
                      header: str, body: list[str]) -> None:
    text = "\n".join((
        "\\begin{table}[H]",
        "\\centering",
        f"\\caption{{{caption}}}",
        f"\\label{{{label}}}",
        "\\small",
        "\\rowcolors{2}{black!3}{white}",
        "\\begin{adjustbox}{max width=\\linewidth}",
        f"\\begin{{tabular}}{{{columns}}}",
        "\\toprule",
        header + "\\\\",
        "\\midrule",
        *(line + "\\\\" for line in body),
        "\\bottomrule",
        "\\end{tabular}",
        "\\end{adjustbox}",
        "\\rowcolors{2}{}{}",
        "\\end{table}",
        "",
    ))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def publish_paper(paper_dir: Path, summaries: list[dict[str, str]]) -> None:
    by_case = {row["case"]: row for row in summaries}
    required = set(CASE_ORDER)
    missing = sorted(required.difference(by_case))
    if missing:
        raise RuntimeError(f"paper publication requires all cases, missing: {missing}")

    data_fields = [
        "case", "repetitions", "status", "optimal_response_cost", "validated",
        "fresh_cycles", "fresh_requests", "fresh_max_response", "fresh_max_active_domain",
        "arena_states", "arena_edges", "groundings", "progressions", "isomorphism_checks",
        "isomorphism_matches", "greedy_upper_bound", "kmax", "arena_ms_median", "arena_ms_iqr",
        "synthesis_ms_median", "synthesis_ms_iqr", "peak_rss_kb_median", "peak_rss_kb_iqr",
    ]
    paper_rows = [{field: row[field] for field in data_fields} for row in summaries]
    write_tsv(paper_dir / "data" / "evaluation_summary.tsv", paper_rows, data_fields)

    faithful_time = by_case["faithful-breadth-first"]["arena_ms_median"]
    faithful_iqr = by_case["faithful-breadth-first"]["arena_ms_iqr"]
    scaling_rows = []
    for identifiers, name in ((2, "finite-explicit"), (3, "finite-explicit-3"), (4, "finite-explicit-4")):
        row = by_case[name]
        scaling_rows.append({
            "identifiers": str(identifiers),
            "explicit_ms": row["arena_ms_median"],
            "explicit_iqr": row["arena_ms_iqr"],
            "faithful_ms": faithful_time,
            "faithful_iqr": faithful_iqr,
            "explicit_states": row["arena_states"],
            "faithful_states": by_case["faithful-breadth-first"]["arena_states"],
        })
    write_tsv(paper_dir / "data" / "evaluation_scaling.tsv", scaling_rows, list(scaling_rows[0]))

    correctness_names = ("finite-explicit", "faithful-breadth-first", "faithful-lower-cost-first", "faithful-greedy")
    correctness_body = []
    for name in correctness_names:
        row = by_case[name]
        correctness_body.append(" & ".join((
            LABELS[name],
            "yes" if row["status"] == "winning" else "no",
            row["optimal_response_cost"],
            "yes" if row["validated"] == "1" else "no",
            row["arena_states"],
            row["arena_edges"],
        )))
    write_latex_table(
        paper_dir / "tables" / "evaluation_correctness.tex",
        "Exact outcomes and solver/extractor-independent replay against a regenerated arena. All worklist orders close the arena and preserve the optimum.",
        "tab:exact-correctness", "lrrrrr",
        "Backend / order & Winning & $K^*$ & Valid & States & Edges", correctness_body)

    representation_body = []
    for name in ("finite-explicit", "faithful-breadth-first"):
        row = by_case[name]
        representation_body.append(" & ".join((
            LABELS[name], row["groundings"], row["progressions"], row["isomorphism_checks"],
            latex_time(row), latex_memory(row),
        )))
    write_latex_table(
        paper_dir / "tables" / "evaluation_representation.tex",
        "Explicit grounding and faithful abstraction. Bracketed values are interquartile ranges around medians.",
        "tab:representation-results", "lrrrrr",
        "Representation & Substitutions & Progressions & Iso. checks & Arena (ms) & Peak (MiB)", representation_body)

    worklist_body = []
    for name in ("faithful-breadth-first", "faithful-lower-cost-first", "faithful-greedy"):
        row = by_case[name]
        worklist_body.append(" & ".join((
            LABELS[name].removeprefix("Faithful / "), row["arena_states"], row["arena_edges"],
            row["isomorphism_checks"], row["greedy_upper_bound"], row["optimal_response_cost"], latex_time(row),
        )))
    write_latex_table(
        paper_dir / "tables" / "evaluation_worklists.tex",
        "Faithful-abstraction worklist guidance. Bracketed arena-time values are interquartile ranges around medians.",
        "tab:worklist-results", "lrrrrrr",
        "Order & States & Edges & Iso. checks & $K_{ub}$ & $K^*$ & Arena (ms)", worklist_body)


def main() -> None:
    parser = argparse.ArgumentParser(description="Aggregate isolated scs_paper repetitions")
    parser.add_argument("root", type=Path)
    parser.add_argument("--paper-dir", type=Path)
    args = parser.parse_args()
    metric_header, rows = read_runs(args.root)
    raw_fields = ["repetition", *metric_header, "peak_rss_kb", "elapsed_s"]
    rows.sort(key=lambda row: (CASE_ORDER.index(row["case"]), int(row["repetition"])))
    write_tsv(args.root / "raw_metrics.tsv", rows, raw_fields)
    summaries = summarize(metric_header, rows)
    summary_fields = list(summaries[0])
    write_tsv(args.root / "summary.tsv", summaries, summary_fields)
    if args.paper_dir:
        publish_paper(args.paper_dir.resolve(), summaries)


if __name__ == "__main__":
    main()
