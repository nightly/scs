# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "marimo>=0.24.0",
# ]
# ///

import marimo

__generated_with = "0.24.0"
app = marimo.App(width="full")


@app.cell
def _(dashboard):
    dashboard
    return


@app.cell
def _():
    import sys
    from pathlib import Path

    repo_root = Path(__file__).resolve().parents[1]
    if str(repo_root) not in sys.path:
        sys.path.insert(0, str(repo_root))

    from src.stage4_explorer_data import empty_record, load_records
    from src.stage4_explorer_ui import (
        build_dashboard,
        corpus_summary,
        filter_records,
        make_detail_controls,
        make_global_controls,
        make_task_picker,
    )

    task_records, illustrative_records = load_records(repo_root / "data" / "stage4")
    domains = sorted({_record["domain"] for _record in task_records})
    families = sorted({_record["family"] for _record in task_records})
    return (
        build_dashboard,
        corpus_summary,
        domains,
        empty_record,
        families,
        filter_records,
        illustrative_records,
        make_detail_controls,
        make_global_controls,
        make_task_picker,
        task_records,
    )


@app.cell
def _(domains, families, make_global_controls):
    global_controls = make_global_controls(domains, families)
    return (global_controls,)


@app.cell
def _(filter_records, global_controls, task_records):
    filtered_records = filter_records(task_records, global_controls)
    return (filtered_records,)


@app.cell
def _(filtered_records, make_task_picker):
    task_picker = make_task_picker(filtered_records)
    return (task_picker,)


@app.cell
def _(empty_record, make_detail_controls, task_picker, task_records):
    if task_picker.value is None:
        selected_record = empty_record()
    else:
        selected_record = next(
            record for record in task_records if record["key"] == task_picker.value
        )
    detail_controls = make_detail_controls(selected_record)
    return detail_controls, selected_record


@app.cell
def _(
    build_dashboard,
    corpus_summary,
    detail_controls,
    filtered_records,
    global_controls,
    illustrative_records,
    selected_record,
    task_picker,
    task_records,
):
    _summary = corpus_summary(task_records)
    dashboard = build_dashboard(
        task_records=task_records,
        illustrative_records=illustrative_records,
        filtered_records=filtered_records,
        selected_record=selected_record,
        global_controls=global_controls,
        task_picker=task_picker,
        detail_controls=detail_controls,
        summary=_summary,
    )
    return (dashboard,)


if __name__ == "__main__":
    app.run()
