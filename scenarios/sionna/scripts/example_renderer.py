"""
Example custom renderer callback for SionnaSceneVisualizer.

This module demonstrates how to create a custom Python renderer callback
that can be used with the SionnaSceneVisualizer's pythonRenderer parameter.

Usage in omnetpp.ini:
    *.physicalEnvironment.sceneVisualizer.pythonRenderer = "scenarios/sionna/scripts/example_renderer.py:render_callback"
"""

import os
import sys
from pathlib import Path

import mitsuba as mi
import numpy as np
import sionna.rt as rt


def render_callback(scene, sim_time, output_dir, frame_index):
    """
    Custom render callback for SionnaSceneVisualizer.

    Parameters:
    -----------
    scene : sionna.rt.Scene
        The Sionna scene object to render
    sim_time : float
        Current simulation time in seconds
    output_dir : str
        Output directory path for rendered frames
    frame_index : int
        Current frame index

    Returns:
    --------
    None
    """

    # Create output directory if it doesn't exist
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    # Example: Create a camera with custom settings
    # You can dynamically adjust camera based on simulation time
    camera = rt.Camera(
        position=mi.Point3f(0.0, 0.0, 150.0),
        orientation=mi.Point3f(0.0, 0.0, 0.0),
    )

    # Example: Adjust rendering parameters based on simulation time
    # This demonstrates the flexibility of using Python callbacks
    if sim_time < 10.0:
        num_samples = 32
        resolution = (1280, 720)
    else:
        num_samples = 64
        resolution = (1920, 1080)

    # Example: Add an environment map for lighting (optional)
    # envmap_path = "/path/to/envmap.exr"
    envmap = None  # Set to a path if you have an envmap

    # Render to file
    filename = output_path / f"frame_{frame_index:06d}.png"

    scene.render_to_file(
        camera=camera,
        filename=str(filename),
        num_samples=num_samples,
        resolution=resolution,
        fov=45.0,
        envmap=envmap,
        lighting_scale=1.0
    )

    print(f"Rendered frame {frame_index} at sim_time {sim_time:.2f}s to {filename}")


def render_callback_with_multiple_cameras(scene, sim_time, output_dir, frame_index):
    """
    Example callback that renders from multiple camera angles.

    This demonstrates how you can render multiple views in a single callback.
    """

    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    # Define multiple camera positions
    cameras = [
        ("front", mi.Point3f(0.0, -100.0, 50.0), mi.Point3f(0.0, 0.1, 0.0)),
        ("top", mi.Point3f(0.0, 0.0, 200.0), mi.Point3f(0.0, 0.0, 0.0)),
        ("side", mi.Point3f(100.0, 0.0, 50.0), mi.Point3f(-0.1, 0.0, 0.0)),
    ]

    for cam_name, position, orientation in cameras:
        camera = rt.Camera(
            position=position,
            orientation=orientation,
        )

        # Create subdirectory for each camera
        cam_dir = output_path / cam_name
        cam_dir.mkdir(exist_ok=True)

        filename = cam_dir / f"frame_{frame_index:06d}.png"

        scene.render_to_file(
            camera=camera,
            filename=str(filename),
            num_samples=32,
            resolution=(1280, 720),
            fov=45.0,
        )

    print(f"Rendered frame {frame_index} from {len(cameras)} camera angles")