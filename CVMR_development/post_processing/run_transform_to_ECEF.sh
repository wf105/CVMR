#!/bin/bash

INPUT="geoinfo.txt"
SCRIPT="./post_processing/convert_to_ECEF.py"
OUTPUT_DIR="3D_ecef"

# Each line: variant_id heading_offset pitch_offset heading_inv pitch_inv chirality
configs=(
  "cream_of_the_crop_data/mirrored/pitch_0/_heading_offset=0_pitch_offset=0_heading_inv=flip_90_pitch_inv=flip_90_chirality=mirrored 0 0 flip_90 flip_90 mirrored"
  "cream_of_the_crop_data/mirrored/pitch_0/_heading_offset=0_pitch_offset=0_heading_inv=flip_90_pitch_inv=flip_180_chirality=mirrored 0 0 flip_90 flip_180 mirrored"
  "cream_of_the_crop_data/mirrored/pitch_0/_heading_offset=0_pitch_offset=0_heading_inv=flip_270_pitch_inv=flip_270_chirality=mirrored 0 0 flip_270 flip_270 mirrored"
  "cream_of_the_crop_data/mirrored/pitch_90/_heading_offset=0_pitch_offset=90_heading_inv=flip_90_pitch_inv=none_chirality=mirrored 0 90 flip_90 none mirrored"
  "cream_of_the_crop_data/mirrored/pitch_180/_heading_offset=0_pitch_offset=180_heading_inv=flip_270_pitch_inv=none_chirality=mirrored 0 180 flip_270 none mirrored"
  "cream_of_the_crop_data/normal/pitch_0/_heading_offset=0_pitch_offset=0_heading_inv=flip_90_pitch_inv=flip_90_chirality=normal 0 0 flip_90 flip_90 normal"
  "cream_of_the_crop_data/normal/pitch_0/_heading_offset=0_pitch_offset=0_heading_inv=flip_90_pitch_inv=flip_180_chirality=normal 0 0 flip_90 flip_180 normal"
  "cream_of_the_crop_data/normal/pitch_0/_heading_offset=0_pitch_offset=0_heading_inv=flip_270_pitch_inv=flip_270_chirality=normal 0 0 flip_270 flip_270 normal"
  "cream_of_the_crop_data/normal/pitch_90/_heading_offset=0_pitch_offset=90_heading_inv=flip_90_pitch_inv=none_chirality=normal 0 90 flip_90 none normal"
  "cream_of_the_crop_data/normal/pitch_180/_heading_offset=0_pitch_offset=180_heading_inv=flip_270_pitch_inv=none_chirality=normal 0 180 flip_270 none normal"
)

mkdir -p "$OUTPUT_DIR"

for config in "${configs[@]}"; do
  read -r variant_id heading_offset pitch_offset heading_inv pitch_inv chirality <<< "$config"

  output_filename=$(basename "$variant_id").txt

  echo "Processing: $variant_id → $OUTPUT_DIR/$output_filename"

  python3 "$SCRIPT" \
    --input "$INPUT" \
    --output_dir "$OUTPUT_DIR" \
    --variant_id "$output_filename" \
    --heading_offset "$heading_offset" \
    --pitch_offset "$pitch_offset" \
    --heading_inv "$heading_inv" \
    --pitch_inv "$pitch_inv" \
    --chirality "$chirality"
done
