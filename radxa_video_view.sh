gst-launch-1.0 udpsrc port=5000 caps='application/x-rtp,media=video,encoding-name=H265,payload=97' ! \
  rtph265depay ! avdec_h265 ! autovideosink sync=false
