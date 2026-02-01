(sleep 5; echo "reboot"; sleep 1) | mavproxy.py --master=udp:127.0.0.1:14571
# Change first number to sleep longer or shorter before reboot command is sent