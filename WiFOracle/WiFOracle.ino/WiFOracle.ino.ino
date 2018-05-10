#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


const char* host = "www.surforacle.net";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String waveRequest()
{
    // The next lines are the code to make the http request!
  HTTPClient http;
  http.begin("http://www.surforacle.net/orbs/007/index.php");
  int httpCode = http.GET();
  String payload = http.getString();
  String wh = getValue(payload, ':', 20);
  wh = getValue(wh, ' ', 1);
  String dP = getValue(payload, ':', 21);
  dP = getValue(dP, ' ', 1);
  String aP = getValue(payload, ':', 22);
  aP = getValue(aP, ' ', 1);
  String wD = getValue(payload, ':', 23);
  String wDl = getValue(wD, '&', 0);
  wDl = getValue(wDl, ' ', 1);
  String wDn = getValue(wD, '&', 0);
  wDn = getValue(wDn, '(', 1);
  wD = wDl + " " + wDn;

  String wT = getValue(payload, ':', 24);
  wT = getValue(wT, '&', 0);
  wT = getValue(wT, ' ', 1);

  Serial.print("Wave High: ");   Serial.println(wh);
  Serial.print("dom Perio: ");   Serial.println(dP);
  Serial.print("avg Perio: ");   Serial.println(aP);
  Serial.print("wave Dire: ");   Serial.println(wD);
  Serial.print("water Tem: ");   Serial.println(wT);
 
  String data = wh + "-" + dP + "-" + aP + "-" + wD + "-" + wT;
  return data;
}

void setup() {
  // put your setup code here, to run once:
  WiFiManager wifiManager;
  wifiManager.autoConnect("WiFOracle");
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);

  Serial.begin(115200);
}

void loop() {
  String data = waveRequest();
  Serial.println(data);
  delay(1000*60*30); //Will wait for 30 minutes
    
}
