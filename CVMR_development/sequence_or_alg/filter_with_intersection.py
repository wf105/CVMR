import numpy as np
import argparse
import os
import glob
import shutil
import logging
from datetime import datetime

def setup_logger(log_dir):
    os.makedirs(log_dir, exist_ok=True)
    log_filename = os.path.join(log_dir, f"camera_intersections_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log")
    logging.basicConfig(level=logging.DEBUG,
                        format='%(asctime)s - %(levelname)s - %(message)s',
                        handlers=[
                            logging.FileHandler(log_filename),
                            logging.StreamHandler()
                        ])
    logging.info("Logger initialized.")
    return log_filename

def load_camera_extrinsics(filepath):
    matrices = []
    with open(filepath, 'r') as f:
        for line_num, line in enumerate(f, start=1):
            values = line.strip().split()
            if len(values) != 16:
                logging.warning(f"Skipping line {line_num} in {filepath} (expected 16 values, got {len(values)})")
                continue
            try:
                matrix = np.array([float(v) for v in values], dtype=np.float64).reshape((4, 4))
                matrices.append(matrix)
            except ValueError as e:
                logging.error(f"Error parsing line {line_num} in {filepath}: {e}")
    return matrices


def extract_camera_origin(matrix):
    R = matrix[:3, :3]
    t = matrix[:3, 3]
    origin = -R.T @ t
    direction = R.T @ np.array([0, 0, 1])
    direction /= np.linalg.norm(direction)
    return origin, direction

'''
#better, but no direction check
def closest_point_between_rays(o1, d1, o2, d2):
    d1 = d1 / np.linalg.norm(d1)
    d2 = d2 / np.linalg.norm(d2)
    dot_prod = np.dot(d1, d2)

    if np.abs(dot_prod) > 0.999999:  # Extremely tolerant to parallel rays
        return None

    w0 = o1 - o2
    a = np.dot(d1, d1)
    b = np.dot(d1, d2)
    c = np.dot(d2, d2)
    d = np.dot(d1, w0)
    e = np.dot(d2, w0)
    denom = a * c - b * b

    if np.abs(denom) < 1e-10:  # Much more tolerant
        return None

    s = (b * e - c * d) / denom
    t = (a * e - b * d) / denom
    p1 = o1 + s * d1
    p2 = o2 + t * d2
    midpoint = (p1 + p2) / 2
    dist = np.linalg.norm(p1 - p2)
    return midpoint, dist
'''


def lines_intersect(o1, d1, o2, d2, tol=1e-6):
    d1 = d1 / np.linalg.norm(d1)
    d2 = d2 / np.linalg.norm(d2)
    w0 = o1 - o2
    cross_d1d2 = np.cross(d1, d2)
    denom = np.linalg.norm(cross_d1d2)**2

    if denom < tol:
        # Lines are parallel or nearly parallel
        return None

    s = (np.dot(np.cross(w0, d2), cross_d1d2)) / denom
    t = (np.dot(np.cross(w0, d1), cross_d1d2)) / denom

    p1 = o1 + s * d1
    p2 = o2 + t * d2
    if np.linalg.norm(p1 - p2) > tol:
        # No exact intersection, lines skew
        return None

    return s, t

def closest_point_between_rays(o1, d1, o2, d2, angle_threshold_deg=90, epsilon=1e-4, tol=1e-6):
    d1 = d1 / np.linalg.norm(d1)
    d2 = d2 / np.linalg.norm(d2)

    w0 = o1 - o2
    cross_d1d2 = np.cross(d1, d2)
    denom_lines = np.linalg.norm(cross_d1d2)**2

    # If lines nearly parallel
    if denom_lines < tol:
        return None

    # Infinite line intersection parameters
    s_line = (np.dot(np.cross(w0, d2), cross_d1d2)) / denom_lines
    t_line = (np.dot(np.cross(w0, d1), cross_d1d2)) / denom_lines

    # Closest points on rays parameters
    a = np.dot(d1, d1)
    b = np.dot(d1, d2)
    c = np.dot(d2, d2)
    d = np.dot(d1, w0)
    e = np.dot(d2, w0)
    denom_rays = a * c - b * b

    if np.abs(denom_rays) < tol:
        return None  # Degenerate

    s_ray = (b * e - c * d) / denom_rays
    t_ray = (a * e - b * d) / denom_rays

    # Reject if infinite lines intersect behind origins AND closest points are at/near origins
    if (s_line < 0 or t_line < 0) and (s_ray <= epsilon or t_ray <= epsilon):
        return None

    # Reject if closest points behind rays (strict)
    if s_ray < 0 or t_ray < 0:
        return None

    p1 = o1 + s_ray * d1
    p2 = o2 + t_ray * d2
    midpoint = (p1 + p2) / 2
    dist = np.linalg.norm(p1 - p2)

    # Angle filtering
    v1 = midpoint - o1
    v2 = midpoint - o2
    angle1 = np.degrees(np.arccos(np.clip(np.dot(v1 / np.linalg.norm(v1), d1), -1, 1)))
    angle2 = np.degrees(np.arccos(np.clip(np.dot(v2 / np.linalg.norm(v2), d2), -1, 1)))

    if angle1 > angle_threshold_deg or angle2 > angle_threshold_deg:
        return None

    return midpoint, dist




def compute_bounding_radius(origins):
    center = np.mean(origins, axis=0)
    radius = max(np.linalg.norm(origin - center) for origin in origins)
    logging.info(f"Bounding center: {center}, radius: {radius}")
    return center, radius

def find_intersecting_pairs(matrices, threshold, debug=False):
    origins_dirs = [extract_camera_origin(mat) for mat in matrices]
    origins = [od[0] for od in origins_dirs]
    center, radius = compute_bounding_radius(origins)

    intersection_radius = radius
    valid_pairs = []
    total = 0
    valid = 0

    for i in range(len(origins_dirs)):
        for j in range(i + 1, len(origins_dirs)):
            o1, d1 = origins_dirs[i]
            o2, d2 = origins_dirs[j]
            result = closest_point_between_rays(o1, d1, o2, d2)
            total += 1
            if result is None:
                if debug:
                    logging.debug(f"Pair ({i}, {j}): rays nearly parallel or degenerate — skipped")
                continue
            midpoint, dist = result
            center_dist = np.linalg.norm(midpoint - center)
            #within_radius = center_dist <= intersection_radius
            #within_radius = True
            # Allow points up to 3× the bounding radius from center
            within_radius = np.linalg.norm(midpoint - center) <= intersection_radius * 30 #badish
            if dist < threshold and within_radius:
                valid += 1
                valid_pairs.append((i, j, midpoint, dist))
                if debug:
                    logging.debug(f"Pair ({i}, {j}): Y dist={dist:.5f}, center_dist={center_dist:.5f}")
            elif debug:
                logging.debug(f"Pair ({i}, {j}): X dist={dist:.5f}, center_dist={center_dist:.5f}")
    ratio = valid / total if total > 0 else 0
    logging.info(f"Total pairs evaluated: {total}, Valid: {valid}, Ratio: {ratio:.4f}")
    return valid_pairs, ratio

def process_file(file_path, output_dir, threshold, required_ratio=0.9, debug=False):
    logging.info(f"Processing file: {file_path}")
    matrices = load_camera_extrinsics(file_path)
    if len(matrices) < 2:
        logging.warning(f"{file_path} → skipped (not enough cameras)")
        return

    pairs, ratio = find_intersecting_pairs(matrices, threshold, debug)

    if ratio >= required_ratio:
        os.makedirs(output_dir, exist_ok=True)
        shutil.copy(file_path, os.path.join(output_dir, os.path.basename(file_path)))
        logging.info(f"[!Y!] {file_path} → copied (intersect ratio: {ratio:.2f})")
    else:
        logging.info(f"[X] {file_path} → skipped (intersect ratio: {ratio:.2f})")

def main():
    parser = argparse.ArgumentParser(description="Filter camera extrinsics based on intersection geometry.")
    parser.add_argument('--input', type=str, help='Path to a single .txt file.')
    parser.add_argument('--input_base', type=str, help='Directory containing .txt files.')
    parser.add_argument('--output_base', type=str, required=True, help='Directory to copy filtered files to.')
    parser.add_argument('--threshold', type=float, default=1.0, help='Max allowed distance between ray intersections.')
    parser.add_argument('--intersect_ratio', type=float, default=0.9, help='Minimum required ratio of intersecting pairs.')
    parser.add_argument('--debug', action='store_true', help='Print debug information for each pair.')
    parser.add_argument('--log_dir', type=str, default="logs", help='Directory to save log files.')

    args = parser.parse_args()
    log_file = setup_logger(args.log_dir)
    logging.info("Starting camera intersection filter process...")

    if args.input:
        process_file(args.input, args.output_base, args.threshold, args.intersect_ratio, args.debug)
    elif args.input_base:
        txt_files = glob.glob(os.path.join(args.input_base, '*.txt'))
        if not txt_files:
            logging.error(f"No .txt files found in: {args.input_base}")
        for f in txt_files:
            process_file(f, args.output_base, args.threshold, args.intersect_ratio, args.debug)
    else:
        logging.error("Error: Specify either --input or --input_base.")
    
    logging.info(f"Process complete. Log saved to: {log_file}")

if __name__ == '__main__':
    main()
