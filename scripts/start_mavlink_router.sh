#!/bin/bash
CONF=/home/radxa/chumbo/config/main.conf

# Make sure UART exists before launching
until [ -e /dev/ttyS2 ]; do
    echo "Waiting for /dev/ttyS2..."
    sleep 0.5
done

echo "Starting mavlink-router..."
/usr/bin/mavlink-routerd -c "$CONF"
