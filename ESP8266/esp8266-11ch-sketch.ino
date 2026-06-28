#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>

const char* defaultSSID = "ESP8266_11CH_Relay_Controller";
const char* defaultPassword = "12345678";

const int relayPins[11] = {16, 5, 4, 0, 2, 14, 12, 13, 15, 3, 1};

ESP8266WebServer server(80);
DNSServer dnsServer;

bool relayState[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
String relayNames[11];
String apSSID;
String apPassword;

const int EEPROM_SIZE = 512;
const int STATE_START = 0;
const int NAME_START = 11;
const int AP_NAME_START = 231;
const int AP_PASS_START = 263;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP8266 Relay Control</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; background: #e9ecef; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(140px, 1fr)); gap: 15px; max-width: 900px; margin: 0 auto; }
    .card { background: white; border-radius: 12px; padding: 15px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); display: flex; flex-direction: column; align-items: center; }
    .btn { padding: 15px 0; width: 100%; font-size: 18px; border: none; border-radius: 8px; cursor: pointer; color: white; transition: 0.2s; font-weight: bold; margin-top: 10px; }
    .btn-on { background: #28a745; box-shadow: 0 4px #1e7e34; }
    .btn-on:active { transform: translateY(4px); box-shadow: none; }
    .btn-off { background: #dc3545; box-shadow: 0 4px #bd2130; }
    .btn-off:active { transform: translateY(4px); box-shadow: none; }
    .name-box { font-size: 14px; color: #495057; cursor: pointer; padding: 5px 10px; border-radius: 4px; background: #f8f9fa; width: 100%; box-sizing: border-box; border: 1px solid transparent; transition: 0.2s; text-align: center; }
    .name-box:hover { border-color: #adb5bd; background: #e9ecef; }
    .name-input { font-size: 14px; padding: 5px 10px; border-radius: 4px; border: 1px solid #007bff; outline: none; width: 100%; box-sizing: border-box; text-align: center; font-family: inherit; }
    .status { margin-top: 20px; font-size: 14px; color: #6c757d; }
    .settings-btn { display: inline-block; margin-top: 10px; padding: 8px 16px; background: #007bff; color: white; text-decoration: none; border-radius: 6px; font-size: 14px; }
    .settings-btn:hover { background: #0056b3; }
    #settings-panel { display: none; margin-top: 20px; padding: 20px; background: white; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); max-width: 400px; margin-left: auto; margin-right: auto; }
    #settings-panel input { padding: 8px; width: 60%; border: 1px solid #ced4da; border-radius: 4px; margin-right: 10px; margin-bottom: 10px; }
    #settings-panel button { padding: 8px 16px; border: none; border-radius: 4px; background: #28a745; color: white; cursor: pointer; }
    #settings-panel button:hover { background: #218838; }
  </style>
</head>
<body>
  <div class="grid">
    %BUTTONS%
  </div>
  <div class="status">Device is running</div>
  <a href="#" class="settings-btn" onclick="toggleSettings()">&#9881; Settings</a>
  
  <div id="settings-panel">
    <h3>WiFi Settings</h3>
    <div><input type="text" id="apName" value="%AP_NAME%" placeholder="WiFi Name"></div>
    <div><input type="text" id="apPass" value="%AP_PASS%" placeholder="WiFi Password"></div>
    <button onclick="saveAPSettings()">Save & Reboot</button>
    <p style="font-size: 12px; color: #6c757d; margin-top: 10px;">Device will reboot after saving.</p>
  </div>

  <script>
    function editName(id, currentName) {
      var box = document.getElementById('name-' + id);
      var input = document.createElement('input');
      input.type = 'text';
      input.className = 'name-input';
      input.value = currentName;
      input.id = 'input-' + id;
      
      box.parentNode.replaceChild(input, box);
      input.focus();
      input.select();
      
      input.addEventListener('blur', function() {
        saveName(id, input.value);
      });
      input.addEventListener('keydown', function(e) {
        if (e.key === 'Enter') {
          input.blur();
        }
      });
    }

    function saveName(id, newName) {
      fetch('/rename?relay=' + id + '&name=' + encodeURIComponent(newName))
      .then(response => response.text())
      .then(data => {
        location.reload();
      });
    }

    function toggleSettings() {
      var panel = document.getElementById('settings-panel');
      panel.style.display = (panel.style.display === 'none' || panel.style.display === '') ? 'block' : 'none';
    }

    function saveAPSettings() {
      var newName = document.getElementById('apName').value;
      var newPass = document.getElementById('apPass').value;
      if (newName.trim() === '') {
        alert('WiFi name cannot be empty!');
        return;
      }
      if (newPass.length < 8) {
        alert('Password must be at least 8 characters long!');
        return;
      }
      fetch('/setapsettings?name=' + encodeURIComponent(newName) + '&pass=' + encodeURIComponent(newPass))
      .then(response => response.text())
      .then(data => {
        alert('Settings saved! Device is restarting. Please reconnect to: ' + newName);
        setTimeout(function() { window.location.href = '/'; }, 2000);
      });
    }
  </script>
</body>
</html>
)rawliteral";

void saveStringToEEPROM(int address, String data, int maxLength) {
  for (int i = 0; i < maxLength; i++) {
    if (i < data.length()) {
      EEPROM.write(address + i, data[i]);
    } else {
      EEPROM.write(address + i, 0);
    }
  }
  EEPROM.commit();
  yield();
}

String readStringFromEEPROM(int address, int maxLength) {
  String data = "";
  for (int i = 0; i < maxLength; i++) {
    char c = EEPROM.read(address + i);
    if (c == 0 || c == 255) break;
    data += c;
  }
  return data;
}

String getHTML() {
  String html = FPSTR(index_html);
  String buttonHTML = "";
  
  for (int i = 0; i < 11; i++) {
    String btnClass = relayState[i] ? "btn btn-on" : "btn btn-off";
    String label = relayState[i] ? "ON" : "OFF";
    String name = relayNames[i];
    name.replace("'", "\\'");
    name.replace("\"", "&quot;");
    
    buttonHTML += "<div class='card'>";
    buttonHTML += "<div id='name-" + String(i) + "' class='name-box' onclick=\"editName(" + String(i) + ", '" + name + "')\">" + name + "</div>";
    buttonHTML += "<button class='" + btnClass + "' onclick=\"fetch('/toggle?relay=" + String(i) + "').then(() => location.reload())\">" + label + "</button>";
    buttonHTML += "</div>";
  }
  
  html.replace("%BUTTONS%", buttonHTML);
  html.replace("%AP_NAME%", apSSID);
  html.replace("%AP_PASS%", apPassword);
  return html;
}

void handleRoot() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.send(200, "text/html", getHTML());
}

void handleToggle() {
  if (server.hasArg("relay")) {
    int relayIndex = server.arg("relay").toInt();
    if (relayIndex >= 0 && relayIndex < 11) {
      relayState[relayIndex] = !relayState[relayIndex];
      digitalWrite(relayPins[relayIndex], relayState[relayIndex] ? LOW : HIGH);
      EEPROM.write(STATE_START + relayIndex, relayState[relayIndex] ? 1 : 0);
      EEPROM.commit();
      yield();
      server.send(200, "text/plain", "OK");
      return;
    }
  }
  server.send(400, "text/plain", "Bad Request");
}

void handleRename() {
  if (server.hasArg("relay") && server.hasArg("name")) {
    int relayIndex = server.arg("relay").toInt();
    String newName = server.arg("name");
    newName.trim();
    if (relayIndex >= 0 && relayIndex < 11 && newName.length() > 0) {
      relayNames[relayIndex] = newName;
      saveStringToEEPROM(NAME_START + (relayIndex * 20), newName, 20);
      server.send(200, "text/plain", "OK");
      return;
    }
  }
  server.send(400, "text/plain", "Bad Request");
}

void handleSetAPSettings() {
  if (server.hasArg("name") && server.hasArg("pass")) {
    String newName = server.arg("name");
    String newPass = server.arg("pass");
    newName.trim();
    newPass.trim();
    if (newName.length() > 0 && newPass.length() >= 8) {
      saveStringToEEPROM(AP_NAME_START, newName, 32);
      saveStringToEEPROM(AP_PASS_START, newPass, 32);
      server.send(200, "text/plain", "OK");
      delay(1000);
      ESP.restart();
      return;
    }
  }
  server.send(400, "text/plain", "Bad Request");
}

void handleNotFound() {
  server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/", true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  delay(10);
  
  for (int i = 0; i < 11; i++) {
    pinMode(relayPins[i], OUTPUT);
    int savedState = EEPROM.read(STATE_START + i);
    relayState[i] = (savedState == 1) ? true : false;
    digitalWrite(relayPins[i], relayState[i] ? LOW : HIGH);
    yield();
    
    String defaultName = "Relay " + String(i + 1);
    relayNames[i] = readStringFromEEPROM(NAME_START + (i * 20), 20);
    if (relayNames[i].length() == 0) {
      relayNames[i] = defaultName;
    }
  }

  apSSID = readStringFromEEPROM(AP_NAME_START, 32);
  if (apSSID.length() == 0) {
    apSSID = defaultSSID;
  }
  
  apPassword = readStringFromEEPROM(AP_PASS_START, 32);
  if (apPassword.length() == 0) {
    apPassword = defaultPassword;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID.c_str(), apPassword.c_str());
  dnsServer.start(53, "*", WiFi.softAPIP());

  delay(1000);

  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/rename", handleRename);
  server.on("/setapsettings", handleSetAPSettings);
  server.onNotFound(handleNotFound);

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  yield();
}
