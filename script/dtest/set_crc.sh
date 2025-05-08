#echo "enable:1 pos:3 a-seed0:0x32 r-seed0:0x23 g-seed0:0x56 b-seed0:0x12" > /sys/kernel/debug/dri/1/crtc-0/CRC
#echo $1
echo "enable:$1 pos:3 a-seed0:0x32 r-seed0:0x23 g-seed0:0x56 b-seed0:0x12" > /sys/kernel/debug/dri/1/crtc-0/CRC