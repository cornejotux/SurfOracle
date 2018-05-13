/* This uses the W1Mos D1 R2 & mini Board
 *  
 */
#include <SPI.h>
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <LiquidCrystal_I2C.h>        //To send the info to a LCD Screen

LiquidCrystal_I2C lcd(0x27,16,2); 

static uint32_t timer;

//flag for saving data
bool shouldSaveConfig = false;

String wH = "00";
double  waveheight=0;
String dP = "00";
String aP = "00";
String wD = "00";
String wT = "00";

// Initialize pins for the color LED
int red=5,green=6,blue=7; //

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

void waveRequest()
{
    // The next lines are the code to make the http request!
  HTTPClient http;
  http.begin("http://www.surforacle.net/orbs/007/index.php");
  int httpCode = http.GET();
  String payload = http.getString();
  wH = getValue(payload, ':', 20);
  wH = getValue(wH, ' ', 1);
  dP = getValue(payload, ':', 21);
  dP = getValue(dP, ' ', 1);
  aP = getValue(payload, ':', 22);
  aP = getValue(aP, ' ', 1);
  wD = getValue(payload, ':', 23);
  String wDl = getValue(wD, '&', 0);
  wDl = getValue(wDl, ' ', 1);
  String wDn = getValue(wD, '&', 0);
  wDn = getValue(wDn, '(', 1);
  wD = wDl + " " + wDn;

  wT = getValue(payload, ':', 24);
  wT = getValue(wT, '&', 0);
  wT = getValue(wT, ' ', 1);
  waveheight = wH.toFloat();

  Serial.print("Wave High: ");   Serial.println(wH);
  Serial.print("dom Perio: ");   Serial.println(dP);
  Serial.print("avg Perio: ");   Serial.println(aP);
  Serial.print("wave Dire: ");   Serial.println(wD);
  Serial.print("water Tem: ");   Serial.println(wT);
}

void setup() {
    lcd.init();
     lcd.backlight();
  // put your setup code here, to run once:
  WiFiManager wifiManager;
  wifiManager.autoConnect("WiFOracle");
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);

  Serial.begin(115200);
}


static void updateLCD()
  {
  lcd.clear();
  lcd.setCursor(0,0);   lcd.print("H:");
  lcd.setCursor(2,0);   lcd.print(wH);
  lcd.setCursor(7,0);   lcd.print("P: ");
  lcd.setCursor(9,0);   lcd.print(dP);
  lcd.setCursor(11,0);  lcd.print("|");
  lcd.setCursor(12,0);  lcd.print(aP);
  lcd.setCursor(0,1);   lcd.print("T:");
  lcd.setCursor(2,1);   lcd.print(wT);
  lcd.setCursor(9,1);   lcd.print(wD);
  lcd.setCursor(7,1);   lcd.print("D:");
  lcd.display();
  Serial.println("LCD updated");
  }

void loop ()
{
  //ether.packetLoop(ether.packetReceive());

 if (millis() > timer)
  {

    Serial.println("\nSending request for page /data/latest_obs/46216.rss");
    Serial.println("Obtaining RSS info...");
    waveRequest();

    if (waveheight != 0)
    {
      timer = millis() + 1000*60*30;
      updateLCD();
      updateColor();
    }
    if (waveheight == 0)
    {
      timer = millis() + 1000;// every 30 secs
    }
  }
}

static void updateColor()
{
  
//The code below lights up RGB module light depending on the wave height
if ((waveheight>=0)&&(waveheight<1))
   {
     analogWrite(red   , 0  );     analogWrite(green , 0  );     analogWrite(blue  , 255);
   }
   if ((waveheight>=1)&&(waveheight<2))
   {
     Serial.println(waveheight);
     analogWrite(red   ,0);     analogWrite(green ,255 );     analogWrite(blue  ,255);
   };
      if ((waveheight>=2)&&(waveheight<3))
   {
     analogWrite(red   ,0);     analogWrite(green ,255 );     analogWrite(blue  ,0);
   };
   if ((waveheight>=3)&&(waveheight<4))
   {
     analogWrite(red   ,255);     analogWrite(green ,255 );     analogWrite(blue  ,0);
   };
   if ((waveheight>=4)&&(waveheight<100))
   {
     analogWrite(red, 255 ); analogWrite(green, 0); analogWrite(blue, 0);
   }
  Serial.println("RGB Color updated");
}
