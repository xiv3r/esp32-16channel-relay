#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <esp_wifi.h>

const char* defaultSSID = "ESP32s3_8CH_Relay_Controller";
const char* defaultPassword = "12345678";

const int relayPins[8] = {1, 2, 3, 4, 5, 6, 7, 8};

WebServer server(80);
DNSServer dnsServer;
Preferences preferences;

bool relayState[8] = {0, 0, 0, 0, 0, 0, 0, 0};      
String relayNames[8];                            
String apSSID;
String apPassword;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32-S3 Relay Control</title>
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

String getHTML() {
  String html = String(FPSTR(index_html));
  String buttonHTML = "";
  
  for (int i = 0; i < 8; i++) { 
    String btnClass = relayState[i] ? "btn btn-on" : "btn btn-off";
    String label = relayState[i] ? "ON" : "OFF";
    String name = relayNames[i];
    
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
  server.send(200, "text/html", getHTML());
}

void handleToggle() {
  if (server.hasArg("relay")) {
    int relayIndex = server.arg("relay").toInt();
    if (relayIndex >= 0 && relayIndex < 8) { 
      relayState[relayIndex] = !relayState[relayIndex];
      digitalWrite(relayPins[relayIndex], relayState[relayIndex] ? LOW : HIGH);
      preferences.putBool(String(relayIndex).c_str(), relayState[relayIndex]);
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
    if (relayIndex >= 0 && relayIndex < 8) { 
      relayNames[relayIndex] = newName;
      preferences.putString(("n" + String(relayIndex)).c_str(), newName);
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
      preferences.putString("apName", newName);
      preferences.putString("apPass", newPass);
      server.send(200, "text/plain", "OK");
      delay(1000);
      ESP.restart();
      return;
    }
  }
  server.send(400, "text/plain", "Bad Request");
}

void handleNotFound() {
  server.send(200, "text/html", getHTML());
}

void setup() {
  preferences.begin("relays", false);

  for (int i = 0; i < 8; i++) {
    pinMode(relayPins[i], OUTPUT);
    relayState[i] = preferences.getBool(String(i).c_str(), false);
    digitalWrite(relayPins[i], relayState[i] ? LOW : HIGH);
    
    String defaultName = "Relay " + String(i + 1);
    relayNames[i] = preferences.getString(("n" + String(i)).c_str(), defaultName);
  }

  apSSID = preferences.getString("apName", defaultSSID);
  apPassword = preferences.getString("apPass", defaultPassword);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID.c_str(), apPassword.c_str());
  
  dnsServer.start(53, "*", WiFi.softAPIP());

  esp_wifi_set_ps(WIFI_PS_NONE);
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
  delay(2);
}
