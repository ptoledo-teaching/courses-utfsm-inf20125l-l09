#!/usr/bin/env bash

set -u

TARGET_DIR="${1:-}"

if [ -z "$TARGET_DIR" ]; then
  echo "Uso: $0 <carpeta-de-tests>"
  exit 2
fi

if [ ! -d "$TARGET_DIR" ]; then
  echo "Error: la carpeta no existe: $TARGET_DIR"
  exit 2
fi

count=0

for ext in out err diff; do
  for f in "$TARGET_DIR"/*."$ext"; do
    [ -f "$f" ] || continue
    rm -f "$f"
    count=$((count + 1))
  done
done

echo "Eliminados $count archivos temporales de $TARGET_DIR"
