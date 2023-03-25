# Build an Test Applicaktion with V4L2 to Show Life Stream from Cam on Screen

## Konfigure

The Video Device is Used ist /dev/Video0

## Build on Debian (Linux with Apt-get)

```bash
wget -O build.sh https://raw.githubusercontent.com/DerKleinePunk/ShowVideoCaptureWithSDL/master/build.sh
chmod +x build.sh
./build.sh
```

## V4L

v4l2-ctl -d /dev/video0 --list-ctrls
Check Video Device

v4l2-ctl --list-device