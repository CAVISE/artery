"""
Defines results' directory structure.
"""

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Optional

import numpy as np
import numpy.typing as npt

JsonObject = dict[str, Any]
EntityId = int | str


def parse_entity_id(value: Any) -> EntityId:
    if isinstance(value, int):
        return value
    if isinstance(value, str) and value.isdigit():
        return int(value)
    return str(value)


@dataclass
class MetricSeries:
    name: str
    ticks: npt.NDArray[np.float64]
    values: npt.NDArray[np.float64]

    @classmethod
    def from_json(cls, payload: JsonObject) -> "MetricSeries":
        samples = payload.get("samples", [])
        ticks = np.array([sample["tick"] for sample in samples], dtype=np.float64)
        values = np.array([sample["value"] for sample in samples], dtype=np.float64)
        return cls(name=payload["name"], ticks=ticks, values=values)

    @property
    def samples(self) -> npt.NDArray[np.float64]:
        return np.column_stack((self.ticks, self.values))


@dataclass
class MetricReport:
    name: str
    display_name: str
    series: dict[str, MetricSeries]

    @classmethod
    def from_json(cls, payload: JsonObject) -> "MetricReport":
        return cls(
            name=payload["metric_name"],
            display_name=payload["display_name"],
            series={
                item["name"]: MetricSeries.from_json(item)
                for item in payload.get("series", [])
            },
        )


@dataclass
class EntityReport:
    info: JsonObject
    metrics: dict[str, MetricReport]

    @classmethod
    def from_json(cls, payload: JsonObject) -> "EntityReport":
        return cls(
            info=payload["info"],
            metrics={
                item["metric_name"]: MetricReport.from_json(item)
                for item in payload.get("metrics", [])
            },
        )

    @property
    def entity_id(self) -> EntityId:
        return parse_entity_id(self.info["entity_id"])


@dataclass
class ModuleReport:
    name: str
    entities: dict[EntityId, EntityReport]

    @classmethod
    def from_json(cls, name: str, payload: JsonObject) -> "ModuleReport":
        return cls(
            name=name,
            entities={
                parse_entity_id(item["info"]["entity_id"]): EntityReport.from_json(item)
                for item in payload.get("entities", [])
            },
        )


@dataclass
class OutputDirectory:
    path: Path

    @property
    def name(self) -> str:
        return self.path.name

    @property
    def report_path(self) -> Path:
        reports = sorted(
            self.path.glob("simulation_output/evaluation_outputs/**/report.json")
        )
        if not reports:
            raise FileNotFoundError(f"no report.json found under {self.path}")

        return reports[0]

    @property
    def modules(self) -> dict[str, ModuleReport]:
        with self.report_path.open() as file:
            payload = json.load(file)

        return {
            name: ModuleReport.from_json(name, module)
            for name, module in payload.items()
            if isinstance(module, dict) and "entities" in module
        }

    def series(
        self,
        module: str,
        metric: str,
        *,
        entity_id: Optional[EntityId] = None,
        series_name: Optional[str] = None,
    ) -> list[MetricSeries]:
        module_report = self.modules[module]
        entities = (
            [module_report.entities[entity_id]]
            if entity_id is not None
            else module_report.entities.values()
        )

        result: list[MetricSeries] = []
        for entity in entities:
            metric_report = entity.metrics[metric]
            if series_name is None:
                result.extend(metric_report.series.values())
            else:
                result.append(metric_report.series[series_name])

        return result


@dataclass
class OutputsDirectory:
    path: Path = Path(__file__).with_name("outputs")

    def __post_init__(self) -> None:
        object.__setattr__(self, "path", Path(self.path))

    @property
    def outputs(self) -> dict[str, OutputDirectory]:
        return {
            path.name: OutputDirectory(path)
            for path in sorted(self.path.iterdir())
            if path.is_dir()
        }

    def output(self, name: str) -> OutputDirectory:
        return self.outputs[name]

    def series(
        self,
        output: str,
        module: str,
        metric: str,
        *,
        entity_id: Optional[EntityId] = None,
        series_name: Optional[str] = None,
    ) -> list[MetricSeries]:
        return self.output(output).series(
            module,
            metric,
            entity_id=entity_id,
            series_name=series_name,
        )
