import os
import hashlib
import argparse
import shutil
from collections import defaultdict

def hash_matrix_file(file_path):
    """Hash the content of a matrix file."""
    with open(file_path, 'r') as f:
        content = f.read().strip()
    return hashlib.md5(content.encode('utf-8')).hexdigest()

def find_unique_files(directory):
    """Return a mapping of hash → first file with that content."""
    hash_to_file = {}
    for fname in sorted(os.listdir(directory)):
        if fname.endswith('.txt'):
            fpath = os.path.join(directory, fname)
            file_hash = hash_matrix_file(fpath)
            if file_hash not in hash_to_file:
                hash_to_file[file_hash] = fpath
    return hash_to_file

def copy_unique_files(unique_files, output_dir):
    """Copy unique files to the output directory."""
    os.makedirs(output_dir, exist_ok=True)
    for src_path in unique_files.values():
        fname = os.path.basename(src_path)
        dst_path = os.path.join(output_dir, fname)
        shutil.copy2(src_path, dst_path)
    return len(unique_files)

def main():
    parser = argparse.ArgumentParser(description="Copy unique matrix .txt files to a new directory.")
    parser.add_argument("input_dir", help="Directory containing matrix .txt files")
    parser.add_argument("output_dir", help="Directory where unique files will be copied")
    args = parser.parse_args()

    if not os.path.isdir(args.input_dir):
        print(f"X Error: '{args.input_dir}' is not a valid directory.")
        return

    unique_files = find_unique_files(args.input_dir)
    count = copy_unique_files(unique_files, args.output_dir)

    print(f"Yes! :)  -Copied {count} unique file(s) to: {args.output_dir}")

if __name__ == "__main__":
    main()
