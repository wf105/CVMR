#!/usr/bin/env python3
"""
Unified pipeline for combo generation, permutation, filtering, intersection, and grouping.
This script replaces the sequence of scripts run in run_pipeline_sweep_and_tidy.sh.
"""
import os
import shutil
import argparse

#useless
def ensure_dir(path):
    os.makedirs(path, exist_ok=True) 

#CMon
def remove_dir(path):
    if os.path.exists(path):
        shutil.rmtree(path)

def generate_unflipped_combos(input_file, output_base):
    """
    Generate 64 unflipped combos from the input geoinfo.txt and write them to output_base directory.
    This is a simplified placeholder for the logic in 64_combo_generator.py.
    """
    print(f"[Stage] Generating unflipped 64 combos: {output_base}")
    ensure_dir(output_base)
    # Read geoinfo.txt
    with open(input_file, 'r') as f:
        lines = [line.strip() for line in f if line.strip()]
    # For demonstration, just create 64 files with some content from geoinfo.txt
    for i in range(64):
        combo_path = os.path.join(output_base, f"combo_{i:02d}.txt")
        with open(combo_path, 'w') as out:
            out.write(f"# Combo {i}\n")
            for line in lines:
                out.write(line + '\n')
    print(f"[Stage] Wrote 64 unflipped combos to {output_base}")

def generate_fully_permuted_combos(input_file, output_base):
    """
    Generate fully permuted combos from the input geoinfo.txt and write them to output_base directory.
    This is a simplified placeholder for the logic in only_unique_perm.py.
    """
    print(f"[Stage] Generating fully permuted combos: {output_base}")
    ensure_dir(output_base)
    # Read geoinfo.txt
    with open(input_file, 'r') as f:
        lines = [line.strip() for line in f if line.strip()]
    # For demonstration, just create 128 files with permuted content
    for i in range(128):
        combo_path = os.path.join(output_base, f"perm_combo_{i:03d}.txt")
        with open(combo_path, 'w') as out:
            out.write(f"# Permuted Combo {i}\n")
            for line in reversed(lines) if i % 2 == 0 else lines:
                out.write(line + '\n')
    print(f"[Stage] Wrote 128 fully permuted combos to {output_base}")

def filter_to_unique(input_dir, output_dir):
    """
    Filter input_dir to unique combos and write to output_dir.
    This is a simplified placeholder for the logic in filter_to_unique.py.
    """
    print(f"[Stage] Filtering to unique: {input_dir} -> {output_dir}")
    ensure_dir(output_dir)
    seen = set()
    for fname in os.listdir(input_dir):
        in_path = os.path.join(input_dir, fname)
        if not os.path.isfile(in_path):
            continue
        with open(in_path, 'r') as f:
            content = f.read()
        if content not in seen:
            seen.add(content)
            out_path = os.path.join(output_dir, fname)
            with open(out_path, 'w') as out:
                out.write(content)
    print(f"[Stage] Wrote {len(seen)} unique combos to {output_dir}")

def filter_with_intersection(input_base, output_base, threshold, intersect_ratio, log_dir):
    """
    Filter combos in input_base by intersection criteria and write to output_base.
    This is a simplified placeholder for the logic in filter_with_intersection.py.
    """
    print(f"[Stage] Filtering with intersection: {input_base} -> {output_base}")
    ensure_dir(output_base)
    ensure_dir(log_dir)
    # For demonstration, just copy every Nth file to output_base
    files = sorted([f for f in os.listdir(input_base) if os.path.isfile(os.path.join(input_base, f))])
    for i, fname in enumerate(files):
        if i % 3 == 0:  # Simulate filtering
            in_path = os.path.join(input_base, fname)
            out_path = os.path.join(output_base, fname)
            with open(in_path, 'r') as fin, open(out_path, 'w') as fout:
                fout.write(fin.read())
    # Write a log file
    with open(os.path.join(log_dir, f"log_{os.path.basename(output_base)}.txt"), 'w') as log:
        log.write(f"Filtered {len(files)} files with threshold={threshold}, intersect_ratio={intersect_ratio}\n")
    print(f"[Stage] Filtered {len(files)//3} combos to {output_base}")

def group_processed_data(input_dir):
    """
    Group processed data by chirality and pitch offset.
    This is a simplified placeholder for the logic in group_processed_data.py.
    """
    print(f"[Stage] Grouping processed data: {input_dir}")
    # For demonstration, just create subfolders and move files based on a dummy rule
    for fname in os.listdir(input_dir):
        if not os.path.isfile(os.path.join(input_dir, fname)):
            continue
        if '0' in fname:
            group = 'chirality_0'
        else:
            group = 'chirality_1'
        group_dir = os.path.join(input_dir, group)
        ensure_dir(group_dir)
        shutil.move(os.path.join(input_dir, fname), os.path.join(group_dir, fname))
    print(f"[Stage] Grouped files in {input_dir}")

def main():
    parser = argparse.ArgumentParser(description="Unified pipeline for combo processing.")
    parser.add_argument('--input', default='geoinfo.txt', help='Input geoinfo file')
    args = parser.parse_args()

    # Output directories
    dirs = {
        'unflipped_64_without_R_combos': 'unflipped_64_without_R_combos',
        'fully_permeded': 'fully_permeded',
        'unique_fully_permeded': 'unique_fully_permeded',
        'unique_noR_combos': 'unique_noR_combos',
        'intersection_filtered_unpermeded': 'intersection_filtered_unpermeded',
        'intersection_filtered_with_permeded': 'intersection_filtered_with_permeded',
        'run_logs': 'run_logs',
    }
    # Create output directories
    for d in dirs.values():
        ensure_dir(d)

    # Pipeline steps
    generate_unflipped_combos(args.input, dirs['unflipped_64_without_R_combos'])
    generate_fully_permuted_combos(args.input, dirs['fully_permeded'])
    filter_to_unique(dirs['unflipped_64_without_R_combos'], dirs['unique_noR_combos'])
    filter_to_unique(dirs['fully_permeded'], dirs['unique_fully_permeded'])
    filter_with_intersection(
        dirs['unique_noR_combos'],
        dirs['intersection_filtered_unpermeded'],
        threshold=180,
        intersect_ratio=0.7,
        log_dir=dirs['run_logs']
    )
    filter_with_intersection(
        dirs['unique_fully_permeded'],
        dirs['intersection_filtered_with_permeded'],
        threshold=225,
        intersect_ratio=0.7,
        log_dir=dirs['run_logs']
    )
    group_processed_data(dirs['intersection_filtered_unpermeded'])
    group_processed_data(dirs['intersection_filtered_with_permeded'])

    # Cleanup
    for d in [
        'unflipped_64_without_R_combos',
        'fully_permeded',
        'unique_fully_permeded',
        'unique_noR_combos',
        'run_logs',
    ]:
        remove_dir(d)

    print("Pipeline complete. Final grouped outputs are in:")
    print(f" - {dirs['intersection_filtered_unpermeded']}/")
    print(f" - {dirs['intersection_filtered_with_permeded']}/")

if __name__ == '__main__':
    main()
