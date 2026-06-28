# Requirements
- ESP32S3
- DUPONT WIRES
- 5v 16 CHANNEL RELAY (2 x 8 channel)

# Installations
> Download the firmware in the release and flash to `0x0`

https://github.com/xiv3r/esp32-16channel-relay/releases/tag/esp32s3-16channel

# Wifi Key
- SSID: `ESP32s3_16CH_Relay_Controller`
- PASS: `12345678`

# 16 Channel GPIO Connections
```
ESP32-S3        16 CHANNEL RELAY

GPIO 1  -------> IN1  Relay1
GPIO 2  -------> IN2  Relay2
GPIO 3  -------> IN3  Relay3
GPIO 4  -------> IN4  Relay4
GPIO 5  -------> IN5  Relay5
GPIO 6  -------> IN6  Relay6
GPIO 7  -------> IN7  Relay7
GPIO 8  -------> IN8  Relay8
GPIO 9  -------> IN9  Relay9
GPIO 10 -------> IN10 Relay10
GPIO 11 -------> IN11 Relay11
GPIO 12 -------> IN12 Relay12
GPIO 13 -------> IN13 Relay13
GPIO 14 -------> IN14 Relay14
GPIO 15 -------> IN15 Relay15
GPIO 16 -------> IN16 Relay16
```
