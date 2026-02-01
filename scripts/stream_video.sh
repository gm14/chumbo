#!/bin/bash

# Note: may need system update to work. rsetup -> System Update

PC_IP=192.168.0.28

# WORKING RTSP
# gst-launch-1.0 -v v4l2src device=/dev/video-camera0 ! queue ! \
#   mpph265enc bps=600000 rc-mode=vbr ! rtspclientsink location=rtsp://localhost:8554/test

# WORKING UDP H265
# gst-launch-1.0 -v v4l2src device=/dev/video-camera0 ! queue ! \
#   mpph265enc bps=1200000 rc-mode=vbr ! rtph265pay pt=97 config-interval=1 ! \
#   udpsink host=$PC_IP port=5000 sync=false

# Attempt 264? Apparently 264 is poorly encoded in this kernel according to radxa. 
#gst-launch-1.0 -v v4l2src device=/dev/video-camera0 ! queue ! \
#  mpph264enc bps=200000 rc-mode=vbr ! rtph264pay pt=97 config-interval=-1 ! \
#  udpsink host=$PC_IP port=8888 sync=false

gst-launch-1.0 -v v4l2src device=/dev/video-camera0 \
  ! queue \
  ! videoconvert \
  ! x264enc tune=zerolatency speed-preset=ultrafast key-int-max=30 sliced-threads=false \
  ! video/x-h264,profile=baseline \
  ! rtph264pay config-interval=1 pt=96 mtu=1400 \
  ! udpsink host=192.168.0.28 port=8888 sync=false

#gst-launch-1.0 -v v4l2src device=/dev/video-camera0 \
#  ! queue \
#  ! videoconvert \
#  ! x264enc sliced-threads=false tune=zerolatency speed-preset=ultrafast key-int-max=30 \
#  ! h264parse config-interval=-1 \
#  ! video/x-h264,stream-format=byte-stream,profile=baseline \
#  ! udpsink host=192.168.0.28 port=8888
