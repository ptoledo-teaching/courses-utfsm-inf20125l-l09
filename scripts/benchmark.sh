#!/usr/bin/env bash

set -u

PROG_V1="${1:-./code/navigation_v1}"
PROG_V2="${2:-./code/navigation_v2}"

for prog in "$PROG_V1" "$PROG_V2"; do
  if [ ! -x "$prog" ]; then
    echo "Error: no se encontro el ejecutable $prog"
    echo "Uso: $0 <ruta-v1> <ruta-v2>"
    exit 1
  fi
done

TMPFILE=$(mktemp)
TIMEFORMAT='%R'

echo "=================================================="
echo " Benchmark: v1 (bubble sort) vs v2 (qsort)"
echo "=================================================="
printf "%-8s  %-14s  %-14s\n" "N" "v1 real (s)" "v2 real (s)"
echo "--------  --------------  --------------"

for N in 1000 2000 4000 8000 16000 32000; do
  LC_NUMERIC=C awk -v n="$N" 'BEGIN { srand(42); print n; for(i=1;i<=n;i++) printf "%.4f\n", rand()*10000 }' > "$TMPFILE"

  T1=$( { time ( "$PROG_V1" < "$TMPFILE" > /dev/null 2>/dev/null ); } 2>&1 )
  T2=$( { time ( "$PROG_V2" < "$TMPFILE" > /dev/null 2>/dev/null ); } 2>&1 )

  printf "%-8d  %-14s  %-14s\n" "$N" "$T1" "$T2"
done

rm -f "$TMPFILE"

echo "=================================================="
echo ""
echo "Para ver detalles de uso de memoria con N=16000:"
echo "  awk -v n=16000 'BEGIN{srand(42);print n;for(i=1;i<=n;i++)printf \"%.4f\\n\",rand()*10000}' | /usr/bin/time -v $PROG_V1"
