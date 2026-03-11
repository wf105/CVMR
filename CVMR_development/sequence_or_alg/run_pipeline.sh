#!/bin/bash

set -e
set -u

# Default values
INPUT_FILE="geoinfo.txt"
OUTPUT_BASE="."

# Parse arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --input)
      INPUT_FILE="$2"
      shift 2
      ;;
    --output)
      OUTPUT_BASE="$2"
      shift 2
      ;;
    *)
      echo "Unknown argument: $1"
      exit 1
      ;;
  esac
done

# Resolve paths to absolute
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INPUT_FILE="$(realpath "$INPUT_FILE")"
OUTPUT_BASE="$(realpath "$OUTPUT_BASE")"

# Ensure output directory exists
mkdir -p "$OUTPUT_BASE"

cd "$OUTPUT_BASE"

mkdir -p unflipped_64_without_R_combos
mkdir -p fully_permeded
mkdir -p unique_fully_permeded
mkdir -p unique_noR_combos
mkdir -p intersection_filtered_unpermeded
mkdir -p intersection_filtered_with_permeded
mkdir -p run_logs

echo "Generating unflipped 64 combos..."
python3 "$SCRIPT_DIR/64_combo_generator.py" --input "$INPUT_FILE" --output_base unflipped_64_without_R_combos/

echo "Generating fully permuted combos..."
python3 "$SCRIPT_DIR/only_unique_perm.py" --input "$INPUT_FILE" --output_base ./fully_permeded/

echo "Filtering unflipped combos to unique..."
python3 "$SCRIPT_DIR/filter_to_unique.py" ./unflipped_64_without_R_combos/ ./unique_noR_combos/

echo "Filtering fully permuted combos to unique..."
python3 "$SCRIPT_DIR/filter_to_unique.py" ./fully_permeded/ ./unique_fully_permeded/

echo "Filtering unique unflipped combos by intersection..."
python3 "$SCRIPT_DIR/filter_with_intersection.py" \
  --input_base unique_noR_combos/ \
  --output_base ./intersection_filtered_unpermeded/ \
  --threshold 180 \
  --intersect_ratio 0.7 \
  --log_dir ./run_logs

echo "Filtering unique fully permuted combos by intersection..."
python3 "$SCRIPT_DIR/filter_with_intersection.py" \
  --input_base unique_fully_permeded/ \
  --output_base ./intersection_filtered_with_permeded/ \
  --threshold 225 \
  --intersect_ratio 0.7 \
  --log_dir ./run_logs

echo "Grouping filtered outputs by chirality and pitch offset..."
python3 "$SCRIPT_DIR/group_processed_data.py" intersection_filtered_unpermeded/
python3 "$SCRIPT_DIR/group_processed_data.py" intersection_filtered_with_permeded/

echo "Cleaning up intermediate directories..."
rm -rf unflipped_64_without_R_combos
rm -rf fully_permeded
rm -rf unique_fully_permeded
rm -rf unique_noR_combos
rm -rf run_logs

echo "Pipeline complete. Final grouped outputs are in:"
echo " - intersection_filtered_unpermeded/"
echo " - intersection_filtered_with_permeded/"
