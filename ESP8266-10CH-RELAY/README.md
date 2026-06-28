# Requirements
- ESP8266 NodeMCU V1.0
- 13 F-F DUPONT WIRES
- 5v 11 CHANNEL RELAY

# Installations
> Download the firmware in the release and flash to `0x0`

https://github.com/xiv3r/esp32-16channel-relay/releases/tag/esp8266-11channel

# Wifi Key
- SSID: `ESP8266_10CH_Relay_Controller`
- PASS: `12345678`

# 11 Channel GPIO Connections
```
ESP8266      10 CHANNEL RELAY

D0  -------> IN1   Relay1
D1  -------> IN2   Relay2
D2  -------> IN3   Relay3
D3  -------> IN4   Relay4
D4  -------> IN5   Relay5
D5  -------> IN6   Relay6
D6  -------> IN7   Relay7
D7  -------> IN8   Relay8
RX  -------> IN9   Relay9
TX  -------> IN10  Relay10
```
