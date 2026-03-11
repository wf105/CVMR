# Bundle Adjustment Visualization in CVMR GUI

This document explains how to visualize bundle adjustment results from the global bundle adjustment pipeline in the CVMR GUI.

## Overview

The GUI now supports loading and visualizing:
- **3D point clouds** from bundle adjustment optimization (shown in **green**)
- **Camera frustums** with optimized poses (shown as white/orange wireframes)
- **Original SIFT point clouds** (shown in **magenta**)

## Quick Start

### 1. Run Bundle Adjustment

First, run the bundle adjustment pipeline to generate NPZ result files:

```bash
python global_bundle_adjustment.py \
    --pose_dir /path/to/poses \
    --match_dir /path/to/matches \
    --output_dir /path/to/ba_results \
    --loss huber \
    --pose_prior_weight 100.0
```

This will create NPZ files like: `global_ba_0000_0049.npz`, `global_ba_0050_0099.npz`, etc.

### 2. Extract NPZ to Text Format

Since the GUI cannot directly read NPZ files, extract them to text format:

```bash
# Extract single file
python extract_npz_to_text.py /path/to/ba_results/global_ba_0000_0049.npz

# Extract all NPZ files in a directory
python extract_npz_to_text.py /path/to/ba_results --batch
```

This creates companion text files:
- `global_ba_0000_0049_points.txt` - 3D points (X Y Z per line)
- `global_ba_0000_0049_cameras.txt` - Camera poses (16 values per line: 4x4 matrix)
- `global_ba_0000_0049_intrinsics.txt` - Camera intrinsics
- `global_ba_0000_0049_stats.txt` - Optimization statistics

### 3. Load in GUI

1. **Compile and run the GUI:**
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ./CameraFrustums
   ```

2. **Load Bundle Adjustment Data:**
   - Click **"Load Bundle Adjustment NPZ..."**
   - Select an NPZ file (e.g., `global_ba_0000_0049.npz`)
   - The GUI will automatically look for the extracted text files

3. **Visualization:**
   - **Green points**: Bundle adjustment optimized 3D points
   - **White/Orange frustums**: Optimized camera poses
   - The view will automatically center on the loaded data

## GUI Controls

### Navigation
- **Mouse drag**: Rotate view
- **Arrow keys**: Pan view (Left/Right/Up/Down)
- **Y/H keys**: Move up/down
- **+/- keys**: Zoom in/out

### Buttons
- **"Load Bundle Adjustment NPZ..."**: Load BA results
- **"Clear Bundle Adjustment Data"**: Remove BA visualization
- **"Load SIFT 3D Pointcloud..."**: Load original SIFT points
- **"Draw Rays"**: Toggle camera ray visualization
- **"Reset View"**: Return to origin view

### Camera Selection
- Use the **"Camera"** dropdown to select which camera frustum to highlight
- Selected camera appears in **orange**, others in **white**

## Data Format Details

### Bundle Adjustment NPZ Contents
The NPZ files contain:
- `Rs`: Rotation matrices (N, 3, 3)
- `ts`: Translation vectors (N, 3)
- `points`: 3D points (M, 3)
- `camera_intrinsics`: Camera intrinsics (N, 3, 3)
- `optimization_stats`: Optimization statistics
- `frame_group`: Frame IDs in this group

### Text File Formats

**Points file (`*_points.txt`):**
```
X1 Y1 Z1
X2 Y2 Z2
...
```

**Cameras file (`*_cameras.txt`):**
```
T11 T12 T13 T14 T21 T22 T23 T24 T31 T32 T33 T34 T41 T42 T43 T44
...
```
(Each line is a flattened 4x4 transformation matrix)

## Coordinate System

- **X axis**: Red (right)
- **Y axis**: Green (up)  
- **Z axis**: Blue (forward)

The bundle adjustment results maintain the same coordinate system as your input poses.

## Troubleshooting

### NPZ Files Not Loading
1. Ensure you've extracted NPZ files using `extract_npz_to_text.py`
2. Check that companion text files exist in the same directory
3. Verify file permissions and paths

### Points Not Visible
1. Use **"Reset View"** to return to origin
2. The GUI auto-centers on loaded data, but you may need to zoom out
3. Check point cloud bounds in the console output

### Camera Frustums Missing
1. Ensure the `*_cameras.txt` file exists and is properly formatted
2. Check that camera poses are reasonable (not all zeros)
3. Verify coordinate system matches your data

### Performance Issues
- Large point clouds (>100K points) may slow rendering
- Consider downsampling points for visualization
- Use the "Clear Bundle Adjustment Data" button to free memory

## Color Coding

| Element | Color | Description |
|---------|-------|-------------|
| Grid | Gray | Reference ground plane |
| Global axes | RGB | World coordinate system |
| BA 3D points | Green | Optimized 3D structure |
| SIFT points | Magenta | Original feature points |
| Selected camera | Orange | Currently selected frustum |
| Other cameras | White | Other camera frustums |
| Camera rays | Yellow | Camera viewing rays (optional) |

## Advanced Usage

### Comparing Before/After
1. Load original SIFT points with **"Load SIFT 3D Pointcloud..."**
2. Load bundle adjustment results with **"Load Bundle Adjustment NPZ..."**
3. Both will be visible simultaneously for comparison

### Multiple Datasets
- Use **"Clear Bundle Adjustment Data"** between datasets
- Only one BA dataset can be loaded at a time
- SIFT points and BA points can coexist

### Export Results
- Use **"Export KRTs..."** to save current camera poses
- Exported files can be used in other applications

## Implementation Notes

The bundle adjustment visualization is implemented in:
- `glwidget.h`: Function declarations and member variables
- `glwidget.cpp`: NPZ loading and rendering logic
- `main.cpp`: GUI button connections
- `extract_npz_to_text.py`: NPZ extraction utility

The GUI expects bundle adjustment data in the coordinate system used by your pose metadata. Ensure consistency between your input poses and the visualization expectations.
