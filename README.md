# 1955 Ultimate Radio upgrade with ESP32 MP3 streaming

Full details here: [https://www.codify.nz/ultimate-radio-1955/](https://www.codify.nz/ultimate-radio-1955/)

Remove the original valve radio components and replace with modern components including:
- Two tweeter speakers
- Two full range speakers
- A subwoofer
- A ZK-HT22 2.1 channel amplifier with bluetooth
- WS2812 RGB LED strips in 8x3 arrangement to use as _tuning_ graphics
- Rotary encoder to control the radio station
- ESP32 wroom with external antenna

The stations are loaded at startup from https://github.com/andyvans/ultimate-radio/blob/main/radio-config.txt.  The first line is the default station index (zero based).
The ESP32 then streams the selected station. If bluetooth is connected then the station LEDs change to a blue horizontal line to indicate radio is not enabled.

MP3 streaming and decoding us done using the impressive [https://github.com/pschatzmann/arduino-audio-tools](https://github.com/pschatzmann/arduino-audio-tools).

![Inside](/images/front.jpg)
![Inside](/images/esp32-i2s.jpg)
![Inside](/images/internals.jpg)
![Inside](/images/back.jpg)

