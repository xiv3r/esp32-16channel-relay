# Requirements
- ESP32 30/38P
- DUPONT WIRES
- 5V 16 CHANNEL RELAY (2 x 8 channel)
- ESP32 Expansion Board

# Installations
> Download the firmware in the release and flash to flash offset `0x0`

# Wifi Key
- SSID: `ESP32_16CH_Relay_Controller`
- PASS: `12345678`

# 16 Channel GPIO Connections
```
ESP32            16 CHANNEL RELAY

GPIO 15 -------> IN1  Relay1
GPIO 2  -------> IN2  Relay2
GPIO 4  -------> IN3  Relay3
GPIO 5  -------> IN4  Relay4
GPIO 18 -------> IN5  Relay5
GPIO 19 -------> IN6  Relay6
GPIO 3  -------> IN7  Relay7
GPIO 1  -------> IN8  Relay8
GPIO 23 -------> IN9  Relay9
GPIO 13 -------> IN10 Relay10
GPIO 14 -------> IN11 Relay11
GPIO 27 -------> IN12 Relay12
GPIO 26 -------> IN13 Relay13
GPIO 25 -------> IN14 Relay14
GPIO 33 -------> IN15 Relay15
GPIO 32 -------> IN16 Relay16
```

# Screenshot

<img src="https://github.com/xiv3r/esp32-16channel-relay/blob/main/.github/workflows/image2.jpg">

<br>

<img src="https://github.com/xiv3r/esp32-16channel-relay/blob/main/.github/workflows/image.jpg">
