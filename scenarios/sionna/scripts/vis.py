# %%

"""
Load and visualize vector output from Sionna.
"""

import sqlite3
from dataclasses import dataclass
from functools import cached_property
from pathlib import Path
from typing import Literal, Any

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Configure paths here.
scenario_dir = Path.cwd().parents[0]
results_dir = scenario_dir / ".results-remote/los-reflection-diffuse/results"
plots_dir = scenario_dir / "scripts" / "plots"

# %%

ResultFile = Literal["vector", "scalar"]


@dataclass
class SimVectors:
    vector_file: Path
    scalar_file: Path

    def table(self, name: str, file: ResultFile = "vector") -> pd.DataFrame:
        db = self.vector_file if file == "vector" else self.scalar_file
        with sqlite3.connect(db) as connection:
            return pd.read_sql_query(f'select * from "{name}"', connection)

    @cached_property
    def vector(self) -> pd.DataFrame:
        return self.table("vector")

    @cached_property
    def vectorAttr(self) -> pd.DataFrame:
        return self.table("vectorAttr")

    @cached_property
    def vectorData(self) -> pd.DataFrame:
        return self.table("vectorData")

    @cached_property
    def scalarAttr(self) -> pd.DataFrame:
        return self.table("scalarAttr", "scalar")

    @cached_property
    def run(self) -> pd.DataFrame:
        return self.table("run")

    @cached_property
    def simtimeExp(self) -> int:
        if self.run.shape[0] != 1:
            raise RuntimeError("simtimeExp is only valid for single-run result files")

        return int(self.run.iloc[0]["simtimeExp"])


class SimResultsReader:
    def __init__(self, results_dir: Path = results_dir):
        self.results_dir = results_dir.resolve()

    def read(self) -> SimVectors:
        vector_files = sorted(self.results_dir.glob("*-vectors.sqlite"))
        scalar_files = sorted(self.results_dir.glob("*-scalars.sqlite"))
        return SimVectors(vector_file=vector_files[0], scalar_file=scalar_files[0])


results = SimResultsReader().read()
names, attrs, vec = results.vector, results.vectorAttr, results.vectorData

vec.shape

# %%

INTERACTION_NAMES = {
    -1: "missing",
    0: "los",
    1: "specular",
    2: "diffuse",
    3: "refraction",
    4: "diffraction",
}


class DataHelpers:

    @classmethod
    def transformSimtime(
        cls, frame: pd.DataFrame, data: SimVectors = results
    ) -> pd.DataFrame:
        frame = frame.copy()
        frame["simtime"] = frame["simtimeRaw"] * np.power(10.0, data.simtimeExp)
        return frame

    @classmethod
    def filterVector(cls, data: SimVectors, ids: pd.Series) -> pd.DataFrame:
        return data.vectorData[data.vectorData["vectorId"].isin(ids)].copy()

    @classmethod
    def withVectorMetadata(
        cls, samples: pd.DataFrame, vectors: pd.DataFrame
    ) -> pd.DataFrame:
        columns = ["vectorId", "moduleName", "vectorName"]
        return samples.merge(vectors[columns], on="vectorId", how="left")

    @classmethod
    def vectorSamples(
        cls, vectors: pd.DataFrame, data: SimVectors = results
    ) -> pd.DataFrame:
        samples = cls.filterVector(data, vectors["vectorId"])
        samples = cls.transformSimtime(samples, data)
        samples = cls.withVectorMetadata(samples, vectors)
        return samples.sort_values(["simtime", "vectorId"])

    @classmethod
    def vectorsByPrefix(cls, data: SimVectors, prefix: str) -> pd.DataFrame:
        return data.vector[data.vector["vectorName"].str.startswith(prefix)].copy()


class VisHelpers:
    textColor = "#E5E7EB"
    gridColor = "#94A3B8"
    interactionColors = {
        "missing": "#F43F5E",
        "los": "#22C55E",
        "specular": "#3B82F6",
        "diffuse": "#F59E0B",
        "refraction": "#14B8A6",
        "diffraction": "#A855F7",
    }

    @classmethod
    def styleAxis(cls, ax: plt.Axes) -> plt.Axes:
        ax.figure.patch.set_alpha(0)
        ax.patch.set_alpha(0)

        ax.title.set_color(cls.textColor)
        ax.xaxis.label.set_color(cls.textColor)
        ax.yaxis.label.set_color(cls.textColor)

        ax.tick_params(colors=cls.textColor)
        ax.grid(True, color=cls.gridColor, alpha=0.22, linewidth=0.8)
        for spine in ax.spines.values():
            spine.set_color(cls.textColor)
            spine.set_alpha(0.35)

        return ax

    @classmethod
    def savePlot(cls, ax: plt.Axes, name: str) -> None:
        plots_dir.mkdir(parents=True, exist_ok=True)
        ax.figure.savefig(plots_dir / name, dpi=200, bbox_inches="tight", transparent=True)

    @classmethod
    def styleLegend(cls, legend: Any) -> None:
        for text in legend.get_texts():
            text.set_color(cls.textColor)


# %%

solved_link_vectors = DataHelpers.vectorsByPrefix(results, "sionna.solvedLinks.")
solved_link_vectors["interaction"] = solved_link_vectors["vectorName"].apply(lambda name: name.split(".")[-1])
solved_link_samples = DataHelpers.vectorSamples(solved_link_vectors)
solved_link_samples = solved_link_samples.merge(
    solved_link_vectors[["vectorId", "interaction"]], on="vectorId", how="left"
)
solved_links = (
    solved_link_samples.pivot_table(index="simtime", columns="interaction", values="value", aggfunc="first")
    .sort_index()
    .rename_axis(index="time_s", columns=None)
)
solved_link_effects = solved_links.drop(columns=["missing"], errors="ignore")
solved_link_cumulative = solved_link_effects.cumsum()

figure, ax = plt.subplots(figsize=(11, 5.5))
for column in solved_link_cumulative.columns:
    ax.plot(
        solved_link_cumulative.index,
        solved_link_cumulative[column],
        label=column,
        color=VisHelpers.interactionColors.get(column),
        linewidth=2.4,
        alpha=0.95,
    )

ax.set_title("Cumulative solved links by strongest path type")
ax.set_xlabel("Simulation time [s]")
ax.set_ylabel("Solved links")

VisHelpers.styleAxis(ax)
legend = ax.legend(frameon=False, ncols=1, loc="center left", bbox_to_anchor=(1.02, 0.5))
VisHelpers.styleLegend(legend)

VisHelpers.savePlot(ax, "solved-links-over-time.png")

# %%

figure, ax = plt.subplots(figsize=(12, 6))
stacked = solved_link_effects[
    [column for column in VisHelpers.interactionColors if column in solved_link_effects.columns]
]

ax.stackplot(
    stacked.index,
    [stacked[column] for column in stacked.columns],
    labels=stacked.columns,
    colors=[VisHelpers.interactionColors[column] for column in stacked.columns],
    alpha=0.82,
)

ax.set_title("Solved link composition")
ax.set_xlabel("Simulation time [s]")
ax.set_ylabel("Solved links")

VisHelpers.styleAxis(ax)
legend = ax.legend(frameon=False, ncols=1, loc="center left", bbox_to_anchor=(1.02, 0.5))
VisHelpers.styleLegend(legend)

VisHelpers.savePlot(ax, "solved-link-composition.png")
