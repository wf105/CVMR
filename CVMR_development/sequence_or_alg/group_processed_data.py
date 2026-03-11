import os
import shutil
import argparse
import re

def parse_arguments():
    parser = argparse.ArgumentParser(description="Regroup .txt files by chirality and pitch offset.")
    parser.add_argument("directory", help="Path to directory containing .txt files")
    return parser.parse_args()

def extract_chirality_and_pitch(filename):
    """Extracts chirality and pitch_offset values from the filename."""
    chirality_match = re.search(r"chirality=(\w+)", filename)
    pitch_match = re.search(r"pitch_offset=(\d+)", filename)

    if chirality_match and pitch_match:
        chirality = chirality_match.group(1)
        pitch_offset = pitch_match.group(1)
        return chirality, pitch_offset
    return None, None

def regroup_files(directory):
    for filename in os.listdir(directory):
        if filename.endswith(".txt"):
            chirality, pitch = extract_chirality_and_pitch(filename)
            if chirality and pitch:
                dest_dir = os.path.join(directory, chirality, f"pitch_{pitch}")
                os.makedirs(dest_dir, exist_ok=True)

                src_path = os.path.join(directory, filename)
                dest_path = os.path.join(dest_dir, filename)

                shutil.move(src_path, dest_path)
                print(f"Moved: {filename} → {dest_path}")

def main():
    args = parse_arguments()
    if not os.path.isdir(args.directory):
        print(f"Error: {args.directory} is not a valid directory.")
        return
    regroup_files(args.directory)

if __name__ == "__main__":
    main()
