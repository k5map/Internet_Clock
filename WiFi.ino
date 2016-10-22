/*
 WiFi module
 */

// ***** Configure WiFi
const char* ssid     = "Pat3net";
const char* password = "";
const char* SSID_HOME = "Pat3net";
const char* SSID_HOME_PSWD = "";
const char* SSID_WORK = "HCP";
const char* SSID_WORK_PSWD = "H1CrushProp";
const char* SSID_MIFI = "MPate-Hotspot";
const char* SSID_MIFI_PSWD = "7139079845";


/**************************************************/
void connectWiFi() {
  Serial.println("\nScanning for WiFi Networks...");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("no WiFi networks found");
    // ??? display message
    while(true);
  }
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    // check to see if found home or work wifi network
    if (WiFi.SSID(thisNet) == SSID_HOME) {
      ssid = SSID_HOME;
      password = SSID_HOME_PSWD;
      break;
    }
    else if (WiFi.SSID(thisNet) == SSID_WORK) {
      ssid = SSID_WORK;
      password = SSID_WORK_PSWD;
      break;
    }
    else if (WiFi.SSID(thisNet) == SSID_MIFI) {
      ssid = SSID_MIFI;
      password = SSID_MIFI_PSWD;
      break;
    }
    // ??? no AP found
  }  

  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  delay(20);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    yield();
  }
  Serial.println();
  Serial.println("CONNECTED!");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal = ");
  Serial.print(WiFi.RSSI());
  Serial.println("dBm");
  Serial.println();
}

