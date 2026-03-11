import numpy as np
from scipy.spatial.transform import Rotation as R
from itertools import product

# All 90°-based angle combinations (in radians)
angle_values = [0, 90, 180, 270]
angles_deg = list(product(angle_values, repeat=3))

# Permutations of Euler axis orders
axis_orders = ['ZYX', 'ZXY', 'YXZ', 'YZX', 'XYZ', 'XZY']

# Chirality options: True = mirrored (i.e., flip x axis)
chiralities = [False, True]

# Optional axis inversion flags (flip axes)
axis_flips = list(product([False, True], repeat=3))

def get_flip_matrix(flip_x, flip_y, flip_z):
    scale = np.diag([
        -1 if flip_x else 1,
        -1 if flip_y else 1,
        -1 if flip_z else 1,
    ])
    return scale

def generate_all_transformations():
    unique_matrices = set()
    for order in axis_orders:
        for angles in angles_deg:
            for flip_flags in axis_flips:
                for mirrored in chiralities:
                    # Convert angles to radians
                    angles_rad = np.radians(angles)

                    # Generate base rotation
                    r = R.from_euler(order, angles_rad)
                    rot_mat = r.as_matrix()

                    # Apply axis flips (simulates "inversions")
                    flip_mat = get_flip_matrix(*flip_flags)
                    rot_mat = flip_mat @ rot_mat

                    # Apply chirality flip (mirror x-axis)
                    if mirrored:
                        chirality_flip = np.diag([-1, 1, 1])
                        rot_mat = chirality_flip @ rot_mat

                    # Optional: homogenize small numerical differences
                    rot_mat = np.round(rot_mat, decimals=6)

                    # Add hashable version to set
                    unique_matrices.add(rot_mat.tobytes())
    
    print(f"Generated {len(unique_matrices)} unique transformation matrices.")
    return unique_matrices

generate_all_transformations()
