# Metadata-Guided Pose Recovery

## Overview
This repository contains the mobile and interactive component of the 3D reconstruction framework. It provides a real-time graphical environment for visualizing, editing, and validating camera poses derived from compromised or incomplete sensor metadata.

## Core Architecture
* **`main.cpp`**: Initializes the `QApplication` and defines the primary graphical window. It establishes the widget layout, controls, and the logic for the interactive interface. This file is responsible for instantiating core objects and connecting signals/slots to ensure data updates trigger immediate rendering in the OpenGL viewport.
* **`gl_widget`**: Acts as the primary support system for the main application, managing data feeders and handlers. It communicates directly with shader files and implements the `paint()` function to execute OpenGL-based graphics. It utilizes Vertex Buffer Objects (VBO), Vertex Array Objects (VAO), and transformation matrices to render frustums, rays, and point clouds.

## Project Components
* **Header Files (`.h`)**: Define the interfaces, data structures, and class definitions for the GUI and rendering engine.
* **Shader Programs**: Implement the rendering pipeline for efficient visualization of computational geometry and image projections.
* **Data & Preprocessing**: Includes raw sensor data and modules to transform GPS/INS inputs into unified coordinate frames (e.g., ECEF or ENU) using the WGS84 ellipsoid.

## Usage Instructions
1. **Interaction**: Use the interface spinboxes to manually manipulate intrinsic and extrinsic matrices for real-time pose adjustment.
2. **Validation**: Project image frames and render sparse point clouds to inspect spatial alignment and consistency.
3. **Inference**: Trigger the pose-hypothesis generation to algorithmically hunt for correct sensor orientations from ambiguous metadata.

## Map of Signals
* **Data Updates**: Signals sent from preprocessing or data generators to trigger object updates.
* **Rendering Calls**: Connections between widget interactions and `paint()`/`update()` calls in the OpenGL context.
* **UI Controls**: Logical links between button/spinbox inputs and matrix transformations.

* Questions?: wffarthing@gmail.com

This research was supported in part by an appointment to the Department of Defense (DOD) Research Participation Program
administered by the Oak Ridge Institute for Science and Education (ORISE) through an interagency agreement between the U.S.
Department of Energy (DOE) and the DOD. ORISE is managed by ORAU under DOE contract number DE-SC0014664. All opinions
expressed in this paper are the author's and do not necessarily reflect the policies and views of DOD, DOE, or ORAU/ORISE.
