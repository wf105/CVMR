import numpy as np

input_path = "real_estate.txt"

with open(input_path, "r") as f:
    lines = [l.strip() for l in f if l.strip()]

lines = lines[1:]  # skip header

# Each line: [timestamp, fx, fy, cx, cy, ... 3x4 pose in cols 6-17 ...]
for idx, line in enumerate(lines):
    cols = line.split()
    # Parse pose: 3x4 camera-to-world, row-major (cols 6-17)
    P = np.array([
        [float(cols[6]), float(cols[7]), float(cols[8]), float(cols[9])],
        [float(cols[10]), float(cols[11]), float(cols[12]), float(cols[13])],
        [float(cols[14]), float(cols[15]), float(cols[16]), float(cols[17])]
    ], dtype=np.float64)
    R = P[:, :3]
    t = P[:, 3]

    # Camera center in world coordinates (for camera-to-world: X = R*x + t, so center is -R^-1 t = -R.T t)
    cam_center = -R.T @ t

    # Convert to world-to-camera [R^T | -R^T t]
    R_wc = R.T
    t_wc = -R.T @ t

    # Compose 4x4 matrix
    M = np.eye(4)
    M[:3, :3] = R_wc
    M[:3, 3] = t_wc

    print(f"Camera {idx}:")
    print("  Camera-to-world 3x4 (from txt):")
    print(P)
    print("  Rotation (R):")
    print(R)
    print("  Translation (t):")
    print(t)
    print("  Camera center in world coords (-R.T @ t):")
    print(cam_center)
    print("  World-to-camera 4x4 (used in .h):")
    print(M)
    print("  Translation part of world-to-camera (should be camera center):")
    print(M[:3,3])
    print("-" * 60)
