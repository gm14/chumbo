while true; do
  echo "=== $(date '+%Y-%m-%d %H:%M:%S') ==="
  for z in /sys/class/thermal/thermal_zone*; do
    printf "%-20s " "$(cat $z/type)"
    awk '{printf "%.1f°C\n", $1/1000}' $z/temp
  done
  echo
  sleep 2
done
