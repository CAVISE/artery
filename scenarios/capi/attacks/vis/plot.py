"""
Generate attack metric plots.
"""

from pathlib import Path
import re

import numpy as np

from .directories import OutputDirectory, OutputsDirectory
from .visual_options import PlotVisualBuilder, PlotVisualOptions, pyplot

IOU_SERIES = (("0.3", "0_3"), ("0.5", "0_5"), ("0.7", "0_7"))
ATTACK_PREFIXES = ("flooding-", "masquerade-", "replay-", "spamming")
ARTERY_NO_ATTACK_NAMES = ("artery-no-attack", "artery-na")


def main() -> None:
    plot_flooding(Path(__file__).with_name("plots"), "flooding-")
    plot_masquerade(Path(__file__).with_name("plots"), "masquerade-")
    plot_replay(Path(__file__).with_name("plots"), "replay-")
    plot_spamming(Path(__file__).with_name("plots"), "spamming")
    plot_attack_heatmaps(Path(__file__).with_name("plots"))


def plot_flooding(
    plots_dir: Path,
    flooding_prefix: str,
    *,
    options: PlotVisualOptions = PlotVisualOptions(),
) -> None:
    plot_attack(
        plots_dir,
        flooding_prefix,
        attack_name="flooding",
        attack_title="Flooding",
        options=options,
    )


def plot_masquerade(
    plots_dir: Path,
    masquerade_prefix: str,
    *,
    options: PlotVisualOptions = PlotVisualOptions(),
) -> None:
    plot_attack(
        plots_dir,
        masquerade_prefix,
        attack_name="masquerade",
        attack_title="Masquerade",
        options=options,
    )


def plot_replay(
    plots_dir: Path,
    replay_prefix: str,
    *,
    options: PlotVisualOptions = PlotVisualOptions(),
) -> None:
    plot_attack(
        plots_dir,
        replay_prefix,
        attack_name="replay",
        attack_title="Replay",
        options=options,
    )


def plot_spamming(
    plots_dir: Path,
    spamming_prefix: str,
    *,
    options: PlotVisualOptions = PlotVisualOptions(),
) -> None:
    plot_attack(
        plots_dir,
        spamming_prefix,
        attack_name="spamming",
        attack_title="Spamming",
        options=options,
    )


def plot_attack(
    plots_dir: Path,
    attack_prefix: str,
    *,
    attack_name: str,
    attack_title: str,
    options: PlotVisualOptions,
) -> None:
    outputs = OutputsDirectory()
    canonical = {
        name: output
        for name, output in outputs.outputs.items()
        if name in ARTERY_NO_ATTACK_NAMES or name.startswith("no-artery")
    }
    attacks = {
        name: output
        for name, output in outputs.outputs.items()
        if name.startswith(attack_prefix)
    }
    if not attacks:
        raise RuntimeError(f"no {attack_name} reports found")

    output_dir = plots_dir / attack_name
    output_dir.mkdir(parents=True, exist_ok=True)
    reports = canonical | attacks

    builder = PlotVisualBuilder(options)
    plot_time_series(
        reports,
        metric="ap_at_iou",
        series_name="ap_iou_0_5",
        attack_prefix=attack_prefix,
        title=f"{attack_title} impact on AP over time",
        output_path=output_dir / f"{attack_name}-ap-time.{options.image_format}",
        options=options,
        builder=builder,
    )
    plot_time_series(
        reports,
        metric="mean_recall_at_iou",
        series_name="mrec_iou_0_5",
        attack_prefix=attack_prefix,
        title=f"{attack_title} impact on mean recall over time",
        output_path=output_dir / f"{attack_name}-recall-time.{options.image_format}",
        options=options,
        builder=builder,
    )
    plot_mean_bars_by_iou(
        reports,
        metric="ap_at_iou",
        series_prefix="ap_iou",
        attack_prefix=attack_prefix,
        title=f"Mean AP by {attack_name} attack",
        output_path=output_dir / f"{attack_name}-ap-mean.{options.image_format}",
        options=options,
        builder=builder,
    )
    plot_mean_bars_by_iou(
        reports,
        metric="mean_recall_at_iou",
        series_prefix="mrec_iou",
        attack_prefix=attack_prefix,
        title=f"Mean recall by {attack_name} attack",
        output_path=output_dir / f"{attack_name}-recall-mean.{options.image_format}",
        options=options,
        builder=builder,
    )


def plot_time_series(
    reports: dict[str, OutputDirectory],
    *,
    metric: str,
    series_name: str,
    attack_prefix: str,
    title: str,
    output_path: Path,
    options: PlotVisualOptions,
    builder: PlotVisualBuilder,
) -> None:
    figure, axis = builder.figure()
    ylabel = next(iter(reports.values())).modules["coperception"].entities["global"].metrics[metric].display_name

    colors = pyplot.get_cmap("tab10")
    for index, (name, report) in enumerate(sorted(reports.items())):
        series = report.modules["coperception"].entities["global"].metrics[metric].series[series_name]
        if name in ARTERY_NO_ATTACK_NAMES:
            label = "artery no attack"
        elif name.startswith("no-artery"):
            label = "no artery"
        else:
            label = re.sub(r"-", " ", name.removeprefix(attack_prefix))
            if not label:
                label = attack_prefix
        axis.plot(
            series.ticks,
            series.values,
            label=label,
            linewidth=options.line_width,
            marker=options.marker,
            color=colors(index),
        )

    builder.apply(figure, axis, title=title, ylabel=ylabel)
    figure.tight_layout()
    figure.savefig(output_path, dpi=options.dpi)
    pyplot.close(figure)


def plot_mean_bars_by_iou(
    reports: dict[str, OutputDirectory],
    *,
    metric: str,
    series_prefix: str,
    attack_prefix: str,
    title: str,
    output_path: Path,
    options: PlotVisualOptions,
    builder: PlotVisualBuilder,
) -> None:
    figure, axis = builder.figure()
    figure.set_size_inches(22.0, 12.0, forward=True)
    ylabel = next(iter(reports.values())).modules["coperception"].entities["global"].metrics[metric].display_name

    names = []
    values_by_iou = {iou_label: [] for iou_label, _ in IOU_SERIES}
    for name, report in sorted(reports.items()):
        if name in ARTERY_NO_ATTACK_NAMES:
            names.append("artery no attack")
        elif name.startswith("no-artery"):
            names.append("no artery")
        else:
            label = re.sub(r"-", " ", name.removeprefix(attack_prefix))
            names.append(label or attack_prefix)
        metric_report = report.modules["coperception"].entities["global"].metrics[metric]
        for iou_label, iou_key in IOU_SERIES:
            series = metric_report.series[f"{series_prefix}_{iou_key}"]
            values_by_iou[iou_label].append(float(np.mean(series.values)))

    positions = np.arange(len(names))
    width = 0.22
    colors = pyplot.get_cmap("tab10")
    for index, (iou_label, _) in enumerate(IOU_SERIES):
        offset = (index - 1) * width
        axis.bar(
            positions + offset,
            values_by_iou[iou_label],
            width=width,
            label=f"{ylabel}@IoU{iou_label}",
            color=colors(index),
        )

    axis.set_xticks(positions)
    axis.set_xticklabels(names, rotation=25, ha="right")

    builder.apply(figure, axis, title=title, ylabel=ylabel, xlabel="Attack")
    if options.legend is not None:
        axis.legend(
            loc="lower center",
            ncol=3,
            fontsize=options.fonts.legend_fontsize,
            framealpha=options.legend.frame_alpha,
        )
    figure.savefig(output_path, dpi=options.dpi)
    pyplot.close(figure)


def plot_attack_heatmaps(
    plots_dir: Path,
    *,
    options: PlotVisualOptions = PlotVisualOptions(),
) -> None:
    outputs = OutputsDirectory()
    reports = {
        name: output
        for name, output in outputs.outputs.items()
        if name in ARTERY_NO_ATTACK_NAMES
        or name.startswith("no-artery")
        or name.startswith(ATTACK_PREFIXES)
    }
    baseline_name = next((name for name in ARTERY_NO_ATTACK_NAMES if name in reports), None)
    if baseline_name is None:
        raise RuntimeError("artery no attack report is required for degradation heatmap")

    names = sorted(reports)
    labels = [attack_label(name) for name in names]
    metric_columns = [(f"AP@IoU{iou}", "ap_at_iou", f"ap_iou_{key}") for iou, key in IOU_SERIES]
    metric_columns.extend((f"Recall@IoU{iou}", "mean_recall_at_iou", f"mrec_iou_{key}") for iou, key in IOU_SERIES)

    values = np.array(
        [
            [
                float(np.mean(reports[name].modules["coperception"].entities["global"].metrics[metric].series[series].values))
                for _, metric, series in metric_columns
            ]
            for name in names
        ],
        dtype=np.float64,
    )
    baseline = values[names.index(baseline_name)]
    degradation = baseline - values

    output_dir = plots_dir / "summary"
    output_dir.mkdir(parents=True, exist_ok=True)
    plot_heatmap(
        values,
        row_labels=labels,
        column_labels=[column for column, _, _ in metric_columns],
        title="Attack metric quality",
        colorbar_label="Mean metric value",
        output_path=output_dir / f"all-attacks-quality-heatmap.{options.image_format}",
        options=options,
        cmap="YlGn",
    )
    attack_rows = [
        index
        for index, name in enumerate(names)
        if name not in ARTERY_NO_ATTACK_NAMES and not name.startswith("no-artery")
    ]
    plot_heatmap(
        degradation[attack_rows],
        row_labels=[labels[index] for index in attack_rows],
        column_labels=[column for column, _, _ in metric_columns],
        title="Metric difference from artery no attack",
        colorbar_label="No-attack metric minus scenario metric",
        output_path=output_dir / f"all-attacks-degradation-heatmap.{options.image_format}",
        options=options,
        cmap="RdBu_r",
    )


def plot_heatmap(
    values: np.ndarray,
    *,
    row_labels: list[str],
    column_labels: list[str],
    title: str,
    colorbar_label: str,
    output_path: Path,
    options: PlotVisualOptions,
    cmap: str,
) -> None:
    figure, axis = pyplot.subplots(figsize=(12.0, max(7.0, 0.55 * len(row_labels))))
    figure.patch.set_facecolor(options.colors.background)
    axis.set_facecolor(options.colors.axes_face)

    image = axis.imshow(values, aspect="auto", cmap=cmap)
    colorbar = figure.colorbar(image, ax=axis)
    colorbar.set_label(colorbar_label, fontsize=options.fonts.label_fontsize, color=options.colors.axis_text)
    colorbar.ax.tick_params(labelsize=options.fonts.tick_fontsize, colors=options.colors.axis_text)

    axis.set_title(title, fontsize=options.fonts.title_fontsize, color=options.colors.axis_text, pad=14)
    axis.set_xticks(np.arange(len(column_labels)))
    axis.set_xticklabels(column_labels, rotation=30, ha="right")
    axis.set_yticks(np.arange(len(row_labels)))
    axis.set_yticklabels(row_labels)
    axis.tick_params(axis="both", labelsize=11, colors=options.colors.axis_text)

    for row in range(values.shape[0]):
        for column in range(values.shape[1]):
            axis.text(
                column,
                row,
                f"{values[row, column]:.3f}",
                ha="center",
                va="center",
                fontsize=13,
                color="#111827",
            )

    figure.tight_layout()
    figure.savefig(output_path, dpi=options.dpi)
    pyplot.close(figure)


def attack_label(name: str) -> str:
    if name in ARTERY_NO_ATTACK_NAMES:
        return "artery no attack"
    if name.startswith("no-artery"):
        return "no artery"
    if name.startswith("flooding-random-"):
        value = name.removeprefix("flooding-random-").replace("0.", "")
        return f"flooding r{value}"
    if name.startswith("flooding-periods-"):
        value = name.removeprefix("flooding-periods-")
        return f"flooding p{value}"
    if name.startswith("masquerade-"):
        value = name.removeprefix("masquerade-")
        if value == "random":
            value = "rand"
        return f"masquerade {value}"
    if name.startswith("replay-repeat-"):
        return f"replay repeat{name.removeprefix('replay-repeat-')}"
    if name.startswith("replay-first-"):
        value = name.removeprefix("replay-first-").removeprefix("cav-")
        return f"replay first {value}"
    if name == "spamming":
        return "spamming"
    return re.sub(r"-", " ", name)


if __name__ == "__main__":
    main()
