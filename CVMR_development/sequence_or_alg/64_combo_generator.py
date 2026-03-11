import numpy as np
import pandas as pd
from pyproj import Transformer
import os
from itertools import product

def deg2rad(deg):
    return deg * np.pi / 180.0

def rotation_matrix(h, p, r, order='ZYX'):
    h, p, r = deg2rad(h), deg2rad(p), deg2rad(r)

    Rx = np.array([
        [1, 0, 0],
        [0, np.cos(r), -np.sin(r)],
        [0, np.sin(r),  np.cos(r)]
    ])
    Ry = np.array([
        [ np.cos(p), 0, np.sin(p)],
        [0, 1, 0],
        [-np.sin(p), 0, np.cos(p)]
    ])
    Rz = np.array([
        [np.cos(h), -np.sin(h), 0],
        [np.sin(h),  np.cos(h), 0],
        [0, 0, 1]
    ])
    return Rz @ Ry @ Rx  # Yaw-Pitch-Roll

def geodetic_to_ecef(lat, lon, alt):
    transformer = Transformer.from_crs("epsg:4326", "epsg:4978", always_xy=True)
    x, y, z = transformer.transform(lon, lat, alt)
    return np.array([x, y, z])

def ecef_to_enu_matrix(ref_lat, ref_lon):
    lat, lon = deg2rad(ref_lat), deg2rad(ref_lon)
    sin_lat, cos_lat = np.sin(lat), np.cos(lat)
    sin_lon, cos_lon = np.sin(lon), np.cos(lon)

    return np.array([
        [-sin_lon,            cos_lon,           0],
        [-sin_lat*cos_lon, -sin_lat*sin_lon, cos_lat],
        [ cos_lat*cos_lon,  cos_lat*sin_lon, sin_lat]
    ])

# 4 inversion variants
inversion_types = {
    "none": lambda x: x % 360,
    "flip_90": lambda x: (90 - x) % 360,
    "flip_180": lambda x: (180 - x) % 360,
    "flip_270": lambda x: (270 - x) % 360,
}

# 2 chiralities
chiralities = {
    "normal": lambda R: R,
    "mirrored": lambda R: R @ np.diag([1, -1, 1]),
}

def process_with_permutations(input_txt, output_base):
    df = pd.read_csv(input_txt, delim_whitespace=True)
    os.makedirs(os.path.dirname(output_base), exist_ok=True)

    # Convert lat/lon/alt to ECEF
    positions_ecef = np.array([
        geodetic_to_ecef(lat, lon, alt)
        for lat, lon, alt in zip(df.latitude, df.longitude, df.altitude)
    ])

    # ENU conversion
    mean_lat, mean_lon = df.latitude.mean(), df.longitude.mean()
    origin_ecef = geodetic_to_ecef(mean_lat, mean_lon, 0.0)
    enu_matrix = ecef_to_enu_matrix(mean_lat, mean_lon)
    enu_positions = (enu_matrix @ (positions_ecef - origin_ecef).T).T
    #enu_positions[:, 2] *= -1  # Z-up for GUI      #CONSIDER----------------

    heading_offsets = [0, 90, 180, 270]
    pitch_offsets = [0, 90, 180, 270]

    for (
        heading_offset,
        pitch_offset,
        heading_inv,
        pitch_inv,
        chirality
    ) in product(
        heading_offsets,
        pitch_offsets,
        inversion_types.items(),
        inversion_types.items(),
        chiralities.items()
    ):
        heading_inv_label, heading_inv_fn = heading_inv
        pitch_inv_label, pitch_inv_fn = pitch_inv
        chirality_label, chirality_fn = chirality

        variant_id = (
            f"heading_offset={heading_offset}"
            f"_pitch_offset={pitch_offset}"
            f"_heading_inv={heading_inv_label}"
            f"_pitch_inv={pitch_inv_label}"
            f"_chirality={chirality_label}"
        )
        output_file = f"{output_base}_{variant_id}.txt"

        with open(output_file, 'w') as f:
            for i, row in df.iterrows():
                h = heading_inv_fn((row.heading + heading_offset) % 360)
                p = pitch_inv_fn((row.pitch + pitch_offset) % 360)
                r = row.roll

                R = rotation_matrix(h, p, r)
                R = chirality_fn(R)

                #pos = enu_positions[i] + np.array([0, 0, -2500.0])         #CONSIDER---------------
                pos = enu_positions[i]
                M = np.eye(4)
                M[:3, :3] = R
                M[:3, 3] = pos

                mat_str = ' '.join(f'{v:.8f}' for v in M.flatten())
                f.write(f"{mat_str}\n")

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Generate 512 pose permutations.")
    parser.add_argument("--input", type=str, required=True, help="Input TXT file with camera pose info")
    parser.add_argument("--output_base", type=str, required=True, help="Base path for output matrices")
    args = parser.parse_args()

    process_with_permutations(args.input, args.output_base)