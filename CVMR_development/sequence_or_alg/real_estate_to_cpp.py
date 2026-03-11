import numpy as np

input_path = "00a907a68edf05c7.txt"
output_path = "real_estate_camera_preprocessor.h"

with open(input_path, "r") as f:
    lines = [l.strip() for l in f if l.strip()]

# Skip the first line ([video_link])
lines = lines[1:]

header = """#pragma once
#include <QMatrix4x4>
#include <vector>

// Auto-generated from real_estate.txt
// Each QMatrix4x4 is a world-to-camera (OpenGL style) matrix, column-major.
// Assumes real_estate.txt pose is camera-to-world, row-major, 3x4.
inline std::vector<QMatrix4x4> realEstateCameraExtrinsics() {
    std::vector<QMatrix4x4> cams;
"""

footer = "    return cams;\n}\n"

body = ""
for line in lines:
    cols = line.split()
    if len(cols) != 19:
        print(f"Skipping malformed line: {line}")
        continue

    # Get the last 12 values as the extrinsic pose (row-major 3x4)
    pose = [float(x) for x in cols[7:19]]
    print("Raw pose (row-major 3x4):", pose)
    P = np.eye(4)
    P[:3, :4] = np.array(pose).reshape(3, 4)
    print("Camera-to-world 4x4 matrix (P):\n", P)

    # Invert to get world-to-camera matrix for OpenGL. Lowkey cap
    P_inv = np.linalg.inv(P)
    print("World-to-camera 4x4 matrix (P_inv):\n", P_inv)

    # Now flatten in column-major order for QMatrix4x4
    #P_inv_T = P_inv.T
    #print("World-to-camera 4x4 matrix, transposed (P_inv.T):\n", P_inv_T)
    vals = P_inv.flatten()  # QMatrix4x4 expects column-major
    print("Flattened values for QMatrix4x4:", vals)
    vals_str = ", ".join(f"{v:.9f}" for v in vals)
    body += f"    cams.emplace_back(QMatrix4x4({vals_str}));\n"

with open(output_path, "w") as f:
    f.write(header)
    f.write(body)
    f.write(footer)

print(f"Wrote {output_path}")