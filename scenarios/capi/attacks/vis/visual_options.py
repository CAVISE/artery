"""
Shared visual options for attack metric plots.
"""

from dataclasses import dataclass, field
from typing import Any, Optional

import matplotlib as mpl
import matplotlib.pyplot as pyplot


@dataclass
class PlotColors:
    """Color palette for metric time-series plots."""

    baseline: str = "#2D6A4F"
    attack: str = "#B42318"
    secondary: str = "#1D4ED8"
    axis_text: str = "#1F2937"
    background: str = "#FFFFFF"
    axes_face: str = "#F8FAFC"
    grid: str = "#CBD5E1"
    statistics_box_face: str = "#FFFFFF"
    statistics_box_edge: str = "#64748B"


@dataclass
class PlotFonts:
    """Typography options for metric time-series plots."""

    font_family: str = "serif"
    title_fontsize: int = 22
    label_fontsize: int = 18
    tick_fontsize: int = 15
    legend_fontsize: int = 13
    statistics_fontsize: int = 13


@dataclass
class PlotStatisticsOptions:
    """Options for the summary statistics box."""

    precision: int = 3
    box_x: float = 0.5
    box_y: float = 0.035
    bottom_margin: float = 0.12
    box_alpha: float = 0.92
    box_style: str = "round,pad=0.45"


@dataclass
class PlotGridOptions:
    """Options for the major grid."""

    alpha: float = 0.45
    linestyle: str = "--"


@dataclass
class PlotLegendOptions:
    """Options for the plot legend."""

    frame_alpha: float = 0.9


@dataclass
class PlotVisualOptions:
    """Visual configuration for metric time-series plots."""

    image_format: str = "png"
    dpi: int = 200
    figsize: tuple[float, float] = (15.0, 6.0)

    line_width: float = 2.0
    marker: Optional[str] = None

    hide_top_spine: bool = True
    hide_right_spine: bool = True
    minor_ticks: bool = True

    colors: PlotColors = field(default_factory=PlotColors)
    fonts: PlotFonts = field(default_factory=PlotFonts)
    grid: Optional[PlotGridOptions] = field(default_factory=PlotGridOptions)
    legend: Optional[PlotLegendOptions] = field(default_factory=PlotLegendOptions)
    statistics: Optional[PlotStatisticsOptions] = field(default_factory=PlotStatisticsOptions)


class PlotVisualBuilder:
    """Applies configured visual options to Matplotlib figures."""

    def __init__(self, options: Optional[PlotVisualOptions] = None) -> None:
        self.options = options or PlotVisualOptions()

    def figure(self) -> tuple[Any, Any]:
        with mpl.rc_context({"font.family": self.options.fonts.font_family}):
            figure, axis = pyplot.subplots(figsize=self.options.figsize)

        self.apply(figure, axis)
        return figure, axis

    def apply(
        self,
        figure: Any,
        axis: Any,
        *,
        title: Optional[str] = None,
        ylabel: Optional[str] = None,
        xlabel: str = "Tick",
        statistics: Optional[str] = None,
    ) -> None:
        figure.patch.set_facecolor(self.options.colors.background)
        axis.set_facecolor(self.options.colors.axes_face)

        axis.spines["top"].set_visible(not self.options.hide_top_spine)
        axis.spines["right"].set_visible(not self.options.hide_right_spine)
        axis.spines["left"].set_color(self.options.colors.axis_text)
        axis.spines["bottom"].set_color(self.options.colors.axis_text)
        axis.spines["left"].set_linewidth(1.1)
        axis.spines["bottom"].set_linewidth(1.1)

        axis.tick_params(
            axis="both",
            labelsize=self.options.fonts.tick_fontsize,
            colors=self.options.colors.axis_text,
            direction="out",
            length=5,
            width=1.0,
        )

        if self.options.minor_ticks:
            axis.minorticks_on()
            axis.tick_params(
                axis="both",
                which="minor",
                length=3,
                width=0.7,
                colors=self.options.colors.axis_text,
            )

        if self.options.grid is not None:
            axis.grid(
                True,
                which="major",
                alpha=self.options.grid.alpha,
                linestyle=self.options.grid.linestyle,
                color=self.options.colors.grid,
            )
            axis.set_axisbelow(True)

        if title is not None:
            axis.set_title(
                title,
                fontsize=self.options.fonts.title_fontsize,
                color=self.options.colors.axis_text,
                pad=14,
            )

        axis.set_xlabel(xlabel, fontsize=self.options.fonts.label_fontsize, color=self.options.colors.axis_text)
        if ylabel is not None:
            axis.set_ylabel(ylabel, fontsize=self.options.fonts.label_fontsize, color=self.options.colors.axis_text)

        if self.options.legend is not None:
            handles, labels = axis.get_legend_handles_labels()
            if handles:
                axis.legend(
                    handles,
                    labels,
                    fontsize=self.options.fonts.legend_fontsize,
                    framealpha=self.options.legend.frame_alpha,
                )

        if self.options.statistics is not None and statistics:
            figure.text(
                self.options.statistics.box_x,
                self.options.statistics.box_y,
                statistics,
                ha="center",
                va="bottom",
                fontsize=self.options.fonts.statistics_fontsize,
                color=self.options.colors.axis_text,
                bbox={
                    "boxstyle": self.options.statistics.box_style,
                    "facecolor": self.options.colors.statistics_box_face,
                    "edgecolor": self.options.colors.statistics_box_edge,
                    "alpha": self.options.statistics.box_alpha,
                },
            )
