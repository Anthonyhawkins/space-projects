#!/usr/bin/env bash

set -euo pipefail

BASE_URL="https://api.starlink.com/public-files/ephemerides"
MANIFEST_URL="$BASE_URL/MANIFEST.txt"
OUTPUT_DIR="./cache"

mkdir -p "$OUTPUT_DIR"

echo "Downloading manifest..."
curl -fsSL "$MANIFEST_URL" -o "$OUTPUT_DIR/MANIFEST.txt"

echo "Processing files..."

while IFS= read -r file; do
    # Skip empty lines
    [[ -z "$file" ]] && continue

    # Remove leading ./ if present
    file="${file#./}"

    local_path="$OUTPUT_DIR/$file"
    file_url="$BASE_URL/$file"

    # Skip if already downloaded
    if [[ -f "$local_path" ]]; then
        echo "Skipping existing: $file"
        continue
    fi

    # Create subdirectories if needed
    mkdir -p "$(dirname "$local_path")"

    echo "Downloading: $file"
    curl -fSL --retry 5 --retry-delay 2 \
        "$file_url" \
        -o "$local_path"

    # Optional polite pause
    sleep 1

done < "$OUTPUT_DIR/MANIFEST.txt"

echo "Done."