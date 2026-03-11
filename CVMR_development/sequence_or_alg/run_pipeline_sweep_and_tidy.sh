#!/bin/bash

set -e  # Exit immediately on error
set -u  # Treat unset variables as an error

# Create output directories
mkdir -p unflipped_64_without_R_combos
mkdir -p fully_permeded
mkdir -p unique_fully_permeded
mkdir -p unique_noR_combos
mkdir -p intersection_filtered_unpermeded
mkdir -p intersection_filtered_with_permeded
mkdir -p run_logs

echo "Generating unflipped 64 combos..."
python3 64_combo_generator.py --input geoinfo.txt --output_base unflipped_64_without_R_combos/

echo "Generating fully permuted combos..."
python3 only_unique_perm.py --input geoinfo.txt --output_base ./fully_permeded/

echo "Filtering unflipped combos to unique..."
python3 filter_to_unique.py ./unflipped_64_without_R_combos/ ./unique_noR_combos/

echo "Filtering fully permuted combos to unique..."
python3 filter_to_unique.py ./fully_permeded/ ./unique_fully_permeded/

echo "Filtering unique unflipped combos by intersection..."
python3 filter_with_intersection.py \
  --input_base unique_noR_combos/ \
  --output_base ./intersection_filtered_unpermeded/ \
  --threshold 180 \
  --intersect_ratio 0.7 \
  --log_dir ./run_logs

echo "Filtering unique fully permuted combos by intersection..."
python3 filter_with_intersection.py \
  --input_base unique_fully_permeded/ \
  --output_base ./intersection_filtered_with_permeded/ \
  --threshold 225 \
  --intersect_ratio 0.7 \
  --log_dir ./run_logs

echo "Grouping filtered outputs by chirality and pitch offset..."
python3 group_processed_data.py intersection_filtered_unpermeded/
python3 group_processed_data.py intersection_filtered_with_permeded/

echo "Cleaning up intermediate directories..."
rm -rf unflipped_64_without_R_combos
rm -rf fully_permeded
rm -rf unique_fully_permeded
rm -rf unique_noR_combos
rm -rf run_logs

echo "Pipeline complete. Final grouped outputs are in:"
echo " - intersection_filtered_unpermeded/"
echo " - intersection_filtered_with_permeded/"
