# Raspberry Pi Audio Setup and Testing Guide

This guide provides step-by-step commands to check and configure the audio output on a **Raspberry Pi** running **Linux**. It includes volume control, audio device selection, and testing audio playback.

## 1. List Available Audio Devices

To see the list of available audio playback devices:
```bash
aplay -l
```

To see the list of all ALSA playback devices:
```bash
aplay -L
```

## 2. Check Current Volume Level

To check the current volume of the headphone output:
```bash
amixer -c 0 get Headphone
```

## 3. Set Volume Level

To set the volume of the headphone output to 20%:
```bash
amixer -c 0 set Headphone 20%
```

## 4. Set Default Audio Output

To test audio output via a specific hardware device (e.g., headphones):
```bash
aplay -D hw:0,0 /usr/share/sounds/alsa/Front_Center.wav
```

If the sound works, set the default audio output by creating or modifying the `~/.asoundrc` file:

```bash
nano ~/.asoundrc
```
Add the following content:
```plaintext
pcm.!default {
  type asym
  playback.pcm {
    type plug
    slave.pcm "output"
  }
  capture.pcm {
    type plug
    slave.pcm "input"
  }
}

pcm.output {
  type hw
  card 0
}

ctl.!default {
  type hw
  card 0
}
```
Save the file (`CTRL+X`, `Y`, `Enter`) and reboot:
```bash
reboot
```

## 5. Install GStreamer ALSA Plugin

If `gst-inspect-1.0 alsasink` returns `No such element or plugin 'alsasink'`, install the required package:
```bash
sudo apt install gstreamer1.0-alsa
```

Then, verify the plugin is installed:
```bash
gst-inspect-1.0 alsasink
```

## 6. Test Audio Playback with GStreamer

To play an MP3 file using GStreamer:
```bash
gst-launch-1.0 filesrc location=/home/pi/audio_test.mp3 ! decodebin ! audioconvert ! audioresample ! alsasink device="hw:0,0"
```

If the playback works, ensure that **ApertusVR** or any other application is using the correct audio device:
```bash
export GST_ALSA_DEVICE="hw:0,0"
./apeSampleLauncher ~/dev/ApertusVR/samples/gstreamer/
```

For debugging GStreamer issues:
```bash
export GST_DEBUG=3
GST_ALSA_DEVICE="hw:0,0" ./apeSampleLauncher ~/dev/ApertusVR/samples/gstreamer/
```

---

This guide ensures that audio output is properly configured and tested on **Raspberry Pi** using **ALSA and GStreamer**. If you experience issues, check `dmesg` or `journalctl -xe` for potential errors.

