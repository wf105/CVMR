import numpy as np
import pandas as pd
from pyproj import Transformer
import os

def deg2rad(deg):
    return deg * np.pi / 180.0

def rotation_matrix(h, p, r):
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
    return Rz @ Ry @ Rx

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

def enu_to_ecef_matrix(ref_lat, ref_lon):
    return ecef_to_enu_matrix(ref_lat, ref_lon).T

# Inversion operations
inversion_ops = {
    "none": lambda x: x % 360,
    "flip_90": lambda x: (90 - x) % 360,
    "flip_180": lambda x: (180 - x) % 360,
    "flip_270": lambda x: (270 - x) % 360,
}

chiralities = {
    "normal": lambda R: R,
    "mirrored": lambda R: R @ np.diag([1, -1, 1]),
}


'''
def apply_transform(
    input_txt,
    heading_offset,
    pitch_offset,
    heading_inv,
    pitch_inv,
    chirality_label,
    variant_id,
    output_dir
):
    os.makedirs(output_dir, exist_ok=True)
    output_file = os.path.join(output_dir, variant_id)
    
    df = pd.read_csv(input_txt, delim_whitespace=True)

    # Setup ENU <-> ECEF transformation
    mean_lat, mean_lon = df.latitude.mean(), df.longitude.mean()
    origin_ecef = geodetic_to_ecef(mean_lat, mean_lon, 0.0)
    enu_matrix = ecef_to_enu_matrix(mean_lat, mean_lon)
    enu_to_ecef_rot = enu_matrix.T

    # Convert positions to ENU
    positions_ecef = np.array([
        geodetic_to_ecef(lat, lon, alt)
        for lat, lon, alt in zip(df.latitude, df.longitude, df.altitude)
    ])
    enu_positions = (enu_matrix @ (positions_ecef - origin_ecef).T).T
    enu_positions[:, 2] *= -1  # Z-up correction

    # Set intrinsics
    K = np.array([
        [500.0,   0.0, 320.0],
        [  0.0, 500.0, 240.0],
        [  0.0,   0.0,   1.0]
    ])

    heading_inv_fn = inversion_ops[heading_inv]
    pitch_inv_fn = inversion_ops[pitch_inv]
    chirality_fn = chiralities[chirality_label]

    with open(output_file, 'w') as f:
        # Write intrinsics
        for row in K:
            f.write(' '.join(f'{v:.8f}' for v in row) + '\n')
        f.write('\n')

        # Apply transformation to each pose
        for i, row in df.iterrows():
            h = heading_inv_fn((row.heading + heading_offset) % 360)
            p = pitch_inv_fn((row.pitch + pitch_offset) % 360)
            r = row.roll

            R_enu = rotation_matrix(h, p, r)
            R_enu = chirality_fn(R_enu)

            # Convert rotation and position to ECEF
            R_ecef = enu_to_ecef_rot @ R_enu
            T_enu = enu_positions[i]
            T_ecef = enu_to_ecef_rot @ T_enu + origin_ecef

            # Write rotation
            for rot_row in R_ecef:
                f.write(' '.join(f'{v:.8f}' for v in rot_row) + '\n')

            # Write translation
            f.write(' '.join(f'{v:.8f}' for v in T_ecef) + '\n\n')

    print(f"Wrote: {output_file}")'''

def apply_transform(
    input_txt,
    heading_offset,
    pitch_offset,
    heading_inv,
    pitch_inv,
    chirality_label,
    variant_id,
    output_dir
):
    os.makedirs(output_dir, exist_ok=True)
    df = pd.read_csv(input_txt, delim_whitespace=True)

    # Setup ENU <-> ECEF transformation
    mean_lat, mean_lon = df.latitude.mean(), df.longitude.mean()
    origin_ecef = geodetic_to_ecef(mean_lat, mean_lon, 0.0)
    enu_matrix = ecef_to_enu_matrix(mean_lat, mean_lon)
    enu_to_ecef_rot = enu_matrix.T

    # Convert positions to ENU
    positions_ecef = np.array([
        geodetic_to_ecef(lat, lon, alt)
        for lat, lon, alt in zip(df.latitude, df.longitude, df.altitude)
    ])
    enu_positions = (enu_matrix @ (positions_ecef - origin_ecef).T).T
    enu_positions[:, 2] *= -1  # Z-up correction

    # Set intrinsics (same for all sensors)
    K = np.array([
        [500.0,   0.0, 320.0],
        [  0.0, 500.0, 240.0],
        [  0.0,   0.0,   1.0]
    ])

    heading_inv_fn = inversion_ops[heading_inv]
    pitch_inv_fn = inversion_ops[pitch_inv]
    chirality_fn = chiralities[chirality_label]

    for i, row in df.iterrows():
        output_file = os.path.join(output_dir, f"{variant_id}_pose_{i:04d}.txt")
        with open(output_file, 'w') as f:
            # Write intrinsics first
            for row_k in K:
                f.write(' '.join(f'{v:.8f}' for v in row_k) + '\n')
            f.write('\n')

            h = heading_inv_fn((row.heading + heading_offset) % 360)
            p = pitch_inv_fn((row.pitch + pitch_offset) % 360)
            r = row.roll

            R_enu = rotation_matrix(h, p, r)
            R_enu = chirality_fn(R_enu)

            R_ecef = enu_to_ecef_rot @ R_enu
            T_enu = enu_positions[i]
            T_ecef = enu_to_ecef_rot @ T_enu + origin_ecef

            # Write rotation
            for rot_row in R_ecef:
                f.write(' '.join(f'{v:.8f}' for v in rot_row) + '\n')
            f.write('\n')

            # Write translation
            f.write(' '.join(f'{v:.8f}' for v in T_ecef) + '\n')

        print(f"Wrote: {output_file}")


'''if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Apply pose transforms and output ECEF KRTs.")
    parser.add_argument("--input", type=str, required=True, help="Input geoinfo file")
    parser.add_argument("--heading_offset", type=float, required=True)
    parser.add_argument("--pitch_offset", type=float, required=True)
    parser.add_argument("--heading_inv", choices=inversion_ops.keys(), required=True)
    parser.add_argument("--pitch_inv", choices=inversion_ops.keys(), required=True)
    parser.add_argument("--chirality", choices=chiralities.keys(), required=True)
    parser.add_argument("--variant_id", type=str, required=True, help="Name for output file")

    args = parser.parse_args()
    apply_transform(
        input_txt=args.input,
        heading_offset=args.heading_offset,
        pitch_offset=args.pitch_offset,
        heading_inv=args.heading_inv,
        pitch_inv=args.pitch_inv,
        chirality_label=args.chirality,
        variant_id=args.variant_id
    )
'''

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Apply pose transforms and output ECEF KRTs.")

    parser.add_argument("--input", type=str, required=True, help="Input geoinfo file (CSV or TXT with lat/lon/alt/heading/pitch/roll)")
    parser.add_argument("--output_dir", type=str, default="3D_ecef", help="Directory to save output KRT files")

    parser.add_argument("--heading_offset", type=float, default=0.0, help="Offset to apply to heading (degrees)")
    parser.add_argument("--pitch_offset", type=float, default=0.0, help="Offset to apply to pitch (degrees)")

    parser.add_argument("--heading_inv", choices=inversion_ops.keys(), default="none", help="Inversion type for heading")
    parser.add_argument("--pitch_inv", choices=inversion_ops.keys(), default="none", help="Inversion type for pitch")

    parser.add_argument("--chirality", choices=chiralities.keys(), default="normal", help="Chirality mode")
    parser.add_argument("--variant_id", type=str, default="transformed_pose", help="Output filename inside output_dir")

    args = parser.parse_args()

    apply_transform(
        input_txt=args.input,
        heading_offset=args.heading_offset,
        pitch_offset=args.pitch_offset,
        heading_inv=args.heading_inv,
        pitch_inv=args.pitch_inv,
        chirality_label=args.chirality,
        variant_id=args.variant_id,
        output_dir=args.output_dir
    )
