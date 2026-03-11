#!/usr/bin/env python3
"""
Extract Bundle Adjustment NPZ files to text format for GUI visualization.
This script reads the NPZ files produced by the global bundle adjustment
and extracts the 3D points and camera poses to simple text files.
"""

import os
import sys
import numpy as np
import argparse
from pathlib import Path

def extract_npz_to_text(npz_path, output_dir=None):
    """
    Extract NPZ file data to text files for GUI loading.
    
    Args:
        npz_path: Path to the NPZ file
        output_dir: Directory to save text files (default: same as NPZ file)
    """
    npz_path = Path(npz_path)
    if not npz_path.exists():
        print(f"Error: NPZ file not found: {npz_path}")
        return False
    
    if output_dir is None:
        output_dir = npz_path.parent
    else:
        output_dir = Path(output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
    
    base_name = npz_path.stem
    
    try:
        # Load NPZ file
        data = np.load(npz_path)
        print(f"Loading NPZ file: {npz_path}")
        print(f"Available keys: {list(data.keys())}")
        
        # Extract 3D points (try 'points', then 'refined_points', then 'original_points')
        points = None
        points_key = None
        for key in ['points', 'refined_points', 'original_points']:
            if key in data:
                points = data[key]
                points_key = key
                break
        if points is not None:
            points_file = output_dir / f"{base_name}_points.txt"
            print(f"Extracting {points.shape[0]} 3D points from '{points_key}' to: {points_file}")
            np.savetxt(points_file, points, fmt='%.6f', delimiter=' ')
            print(f"Points range: X[{points[:, 0].min():.2f}, {points[:, 0].max():.2f}] "
                  f"Y[{points[:, 1].min():.2f}, {points[:, 1].max():.2f}] "
                  f"Z[{points[:, 2].min():.2f}, {points[:, 2].max():.2f}]")
        else:
            print("No 3D point cloud found (no 'points', 'refined_points', or 'original_points' in NPZ)")
        
        # Extract camera poses (rotation matrices and translation vectors)
        if 'Rs' in data and 'ts' in data:
            Rs = data['Rs']  # Rotation matrices (N, 3, 3)
            ts = data['ts']  # Translation vectors (N, 3)
            
            cameras_file = output_dir / f"{base_name}_cameras.txt"
            
            print(f"Extracting {Rs.shape[0]} camera poses to: {cameras_file}")
            
            with open(cameras_file, 'w') as f:
                for i, (R, t) in enumerate(zip(Rs, ts)):
                    # Create 4x4 homogeneous transformation matrix
                    T = np.eye(4)
                    T[:3, :3] = R
                    T[:3, 3] = t
                    
                    # Write as single line (16 values)
                    f.write(' '.join([f'{val:.6f}' for val in T.flatten()]) + '\n')
        
        # Extract camera intrinsics if available
        if 'camera_intrinsics' in data:
            intrinsics = data['camera_intrinsics']
            intrinsics_file = output_dir / f"{base_name}_intrinsics.txt"
            
            print(f"Extracting camera intrinsics to: {intrinsics_file}")
            
            with open(intrinsics_file, 'w') as f:
                for i, K in enumerate(intrinsics):
                    # Write as single line (9 values)
                    f.write(f"# Camera {i}\n")
                    f.write(' '.join([f'{val:.6f}' for val in K.flatten()]) + '\n')
        
        # Extract optimization statistics if available
        if 'optimization_stats' in data:
            stats = data['optimization_stats'].item()
            stats_file = output_dir / f"{base_name}_stats.txt"
            
            print(f"Extracting optimization statistics to: {stats_file}")
            
            with open(stats_file, 'w') as f:
                f.write("Bundle Adjustment Optimization Statistics\n")
                f.write("=" * 50 + "\n")
                for key, value in stats.items():
                    f.write(f"{key}: {value}\n")
        
        # Extract frame group information if available
        if 'frame_group' in data:
            frame_group = data['frame_group']
            group_file = output_dir / f"{base_name}_frame_group.txt"
            
            print(f"Extracting frame group information to: {group_file}")
            
            with open(group_file, 'w') as f:
                f.write("Frame Group: " + ' '.join(map(str, frame_group)) + '\n')
        
        print(f"Extraction completed successfully!")
        return True
        
    except Exception as e:
        print(f"Error extracting NPZ file: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="Extract Bundle Adjustment NPZ to text files for GUI")
    parser.add_argument('npz_file', help='Path to the NPZ file to extract')
    parser.add_argument('--output_dir', '-o', help='Output directory for text files (default: same as NPZ)')
    parser.add_argument('--batch', '-b', action='store_true', help='Process all NPZ files in directory')
    
    args = parser.parse_args()
    
    if args.batch:
        # Process all NPZ files in the directory
        input_dir = Path(args.npz_file)
        if not input_dir.is_dir():
            print(f"Error: {input_dir} is not a directory")
            return 1
        
        npz_files = list(input_dir.glob("*.npz"))
        if not npz_files:
            print(f"No NPZ files found in {input_dir}")
            return 1
        
        print(f"Found {len(npz_files)} NPZ files to process")
        
        success_count = 0
        for npz_file in npz_files:
            print(f"\n--- Processing {npz_file.name} ---")
            if extract_npz_to_text(npz_file, args.output_dir):
                success_count += 1
        
        print(f"\nProcessed {success_count}/{len(npz_files)} files successfully")
        
    else:
        # Process single file
        if not extract_npz_to_text(args.npz_file, args.output_dir):
            return 1
    
    return 0

if __name__ == "__main__":
    exit(main())
