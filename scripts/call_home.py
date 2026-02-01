import socket
import struct
import subprocess
import time

PHONE_PORT = 8888

def find_gateway_ip():
    # Reads the default gateway (Phone's Hotspot IP)
    with open("/proc/net/route") as fh:
        for line in fh:
            fields = line.strip().split()
            if fields[1] != '00000000' or not int(fields[3], 16) & 2:
                continue
            return socket.inet_ntoa(struct.pack("<L", int(fields[2], 16)))

def call_home():
    #gateway_ip = find_gateway_ip()
    gateway_ip = "192.168.0.28"
    print(f"Phone detected at: {gateway_ip}")
    
    # 1. TCP Handshake (Keep this on 8888 or whatever your App expects)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    while True:
        try:
            sock.connect((gateway_ip, 8888)) # App Control Port
            print("Connected to App!")
            break
        except:
            time.sleep(1)

    # 2. Wait for "Start"
    data = sock.recv(1024)
    if "CMD_START_VIDEO" in data.decode():
        print("Starting GStreamer...")
        
        # YOUR WORKING PIPELINE (Wrapped in Python)
        # Note: I swapped $PC_IP for the {gateway_ip} variable
        cmd = (
            f"gst-launch-1.0 -v v4l2src device=/dev/video-camera0 ! queue ! "
            f"mpph265enc bps=1200000 rc-mode=vbr ! "
            f"rtph265pay pt=97 config-interval=1 ! "
            f"udpsink host={gateway_ip} port=5000 sync=false"
        )
        
        subprocess.Popen(cmd, shell=True)

if __name__ == "__main__":
    call_home()
