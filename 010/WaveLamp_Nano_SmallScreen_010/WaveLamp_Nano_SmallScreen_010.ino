#include <LiquidCrystal.h>
#include "EtherCard.h"
//#define DEBUG 1

int red=A5,green=A4,blue=A0; // PWM pins initialized to pass values of RGB
// Connections:
// rs (LCD pin 4) to Arduino pin A3
// rw (LCD pin 5) to Arduino pin A2
// enable (LCD pin 6) to Arduino pin A1
// LCD pin 15 to Arduino pin 13 <-- Creo que este no se usa
// LCD pins d4(11), d5(12), d6(13), d7(14) to Arduino pins 5, 4, 3, 2
LiquidCrystal lcd(A3, A2, A1, 5, 4, 3, 2);

//Connections HR911105A
//CS ->   D8
//ST ->   D11
//SO ->   D12
//SCK -> D13

int backLight = 6; 
 
// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
const char website[] PROGMEM = "www.surforacle.net";
byte Ethernet::buffer[700];
static uint32_t timer;

float waveheight ; // float value that holds wave height
int dPeriod ;
int aPeriod;

String wDirection = "0 00";//= '-9999';
String gDirection ;//= '-9999';
String wTemp ;//= '-9999';
float cTemp;
 
// Patterns and pattern buffer pointer
char *searchPattern[] = { "t:</strong>",
                          "d:</strong> ",
                          "od:</strong>",
                          "n:</strong>",
                          "e:</strong>"};
char *searchPatternProgressPtr;
 
// Output bugger and pointer to the buffer
char displayBuff[64];
char *outputBufferPtr;
 
int foundPatternState;// Initialsed in loop()
 
 
// Utility functions
void removeSubstring(char *s,const char *toremove)
{
  while( s=strstr(s,toremove) )
  {
    memmove(s,s+strlen(toremove),1+strlen(s+strlen(toremove)));
  }
}
 
char *multiBufferFindPattern(char *buffer,char *searchString,char *patternInProgress)
{
    while (*buffer && *patternInProgress)
    {
        if (*buffer == *patternInProgress)
        {
            patternInProgress++;
        }
        else
        {
            patternInProgress=searchString;// reset to start of the pattern
        }
        buffer++;
    }
    if (!*patternInProgress)
    {
        return buffer;
    }
    return NULL;
}
 
int getData(char *inputBuffer, char *outputBuffPtr, char endMarker)
{
    while(*inputBuffer && *inputBuffer!=endMarker && *outputBuffPtr)
    {
        *outputBuffPtr=*inputBuffer;
        outputBuffPtr++;
        inputBuffer++;
 
    }
    if (*inputBuffer==endMarker && *outputBuffPtr!=0)
    {
        *outputBuffPtr=0;
        // end character found
        return 1;
    }
    else
    {
      return 0;
    }
}
 
 
// Called for each packet of returned data from the call to browseUrl (as persistent mode is set just before the call to browseUrl)
static void browseUrlCallback (byte status, word off, word len)
{
  //Serial.println('____m00m_____');
   char *pos;// used for buffer searching
   pos=(char *)(Ethernet::buffer+off);
   Ethernet::buffer[off+len] = 0;// set the byte after the end of the buffer to zero to act as an end marker (also handy for displaying the buffer as a string)
 
   //Serial.println(pos);
   if (foundPatternState==0)
   {
     // initialise pattern search pointers
     searchPatternProgressPtr=searchPattern[0];
     foundPatternState=1;
   }
  
   if (foundPatternState==1)
   {
       pos = multiBufferFindPattern(pos,searchPattern[0],searchPatternProgressPtr);
       if (pos)
       {
         foundPatternState=2;
         outputBufferPtr=displayBuff;
         memset(displayBuff,'0',sizeof(displayBuff));// clear the output display buffer
         displayBuff[sizeof(displayBuff)-1]=0;//end of buffer marker
       } 
       else
       {
         return;// Need to wait for next buffer, so just return to save processing the other if states
       }
   }    
  
   if (foundPatternState==2)
   {
     if (getData(pos,outputBufferPtr,'ft'))
     {
          Serial.print("wH: ");
          removeSubstring(displayBuff," ");// Use utility function to remove unwanted characters
          waveheight = atof(displayBuff);
          Serial.println(waveheight); 
          foundPatternState=3; 
     }
     else
     {
       // end marker is not found, stay in same findPatternState and when the callback is called with the next packet of data, outputBufferPtr will continue where it left off
     }
   }
  if (foundPatternState==3)
   {
        searchPatternProgressPtr=searchPattern[1];
        foundPatternState=4;
   }
   if (foundPatternState==4)
   {
       pos = multiBufferFindPattern(pos,searchPattern[1],searchPatternProgressPtr);
       if (pos)
       {
         foundPatternState=5;
         outputBufferPtr=displayBuff; // Reset outbutBuffertPtr ready to receive new data
         memset(displayBuff,'0',sizeof(displayBuff));// clear the output display buffer
         displayBuff[sizeof(displayBuff)-1]=0;//end of buffer marker
       } 
       else
       {
         return;// Need to wait for next buffer, so just return to save processing the other if states
       }
   }    
  
   if (foundPatternState==5)
   {
     if (getData(pos,outputBufferPtr,' sec'))
     {
          Serial.print("dP: ");
          dPeriod = atof(displayBuff);
          Serial.println(dPeriod);
          foundPatternState=6;  //Move to next state (not used in this demo)
     }
     else
     {
       // end marker is not found, stay in same findPatternState and when the callback is called with the next packet of data, outputBufferPtr will continue where it left off
     }
   }     
  
   if (foundPatternState==6)
   {
        searchPatternProgressPtr=searchPattern[2];
        foundPatternState=7;
   }
   if (foundPatternState==7)
   {
       pos = multiBufferFindPattern(pos,searchPattern[2],searchPatternProgressPtr);
       if (pos)
       {
         foundPatternState=8;
         outputBufferPtr=displayBuff; // Reset outbutBuffertPtr ready to receive new data
         memset(displayBuff,'0',sizeof(displayBuff));// clear the output display buffer
         displayBuff[sizeof(displayBuff)-1]=0;//end of buffer marker
       } 
       else
       {
         return;
       }
   }    
  
   if (foundPatternState==8)
   {
     if (getData(pos,outputBufferPtr,' sec'))
     {
          Serial.print("aP: ");
          aPeriod = atof(displayBuff);
          Serial.println(aPeriod);
          foundPatternState=9;  //Move to next state (not used in this demo)
     }
     else
     {
       // end marker is not found, stay in same findPatternState and when the callback is called with the next packet of data, outputBufferPtr will continue where it left off
     }
   } 
 
   if (foundPatternState==9)
   {
        searchPatternProgressPtr=searchPattern[3];
        foundPatternState=10;
   }
   if (foundPatternState==10)
   {
       pos = multiBufferFindPattern(pos,searchPattern[3],searchPatternProgressPtr);
       if (pos)
       {
         foundPatternState=10;
         outputBufferPtr=displayBuff; // Reset outbutBuffertPtr ready to receive new data
         memset(displayBuff,'0',sizeof(displayBuff));// clear the output display buffer
         displayBuff[sizeof(displayBuff)-1]=0;//end of buffer marker
       } 
       else
       {
         return;
       }
   }    
  
   if (foundPatternState==10)
   {
     if (getData(pos,outputBufferPtr,'&'))
     {
          Serial.print("wD: ");
          removeSubstring(displayBuff,"(");
          wDirection = displayBuff;
          Serial.println(wDirection);
          foundPatternState=11;  //Move to next state (not used in this demo)
     }
     else
     {
       // end marker is not found, stay in same findPatternState and when the callback is called with the next packet of data, outputBufferPtr will continue where it left off
     }
   }


   if (foundPatternState==11)
   {
        searchPatternProgressPtr=searchPattern[4];
        foundPatternState=12;
   }
   if (foundPatternState==12)
   {
       pos = multiBufferFindPattern(pos,searchPattern[4],searchPatternProgressPtr);
       if (pos)
       {
         foundPatternState=13;
         outputBufferPtr=displayBuff; // Reset outbutBuffertPtr ready to receive new data
         memset(displayBuff,'0',sizeof(displayBuff));// clear the output display buffer
         displayBuff[sizeof(displayBuff)-1]=0;//end of buffer marker
       } 
       else
       {
         return;
       }
   }    
  
   if (foundPatternState==13)
   {
     if (getData(pos,outputBufferPtr,'&'))
     {
          Serial.print("wT: ");
          removeSubstring(displayBuff," ");
          int temp = atoi(displayBuff);
          cTemp = temp;// This is for Temp in Celcious(temp - 32) * 5/9;
          wTemp = displayBuff;
          Serial.println(wTemp);
          foundPatternState=999;  //Move to next state (not used in this demo)
     }
     else
     {
       // end marker is not found, stay in same findPatternState and when the callback is called with the next packet of data, outputBufferPtr will continue where it left off
     }
   }
 
   if (foundPatternState==999)
   {
     // Found everything on this page. dissable persistence to stop any more callbacks.
     ether.persistTcpConnection(false);
   }
 }
 
static void updateLCD()
  {
  lcd.clear();
  lcd.setCursor(0,0);   lcd.print("H:");
  lcd.setCursor(2,0);   lcd.print(waveheight,1);
  lcd.setCursor(7,0);  lcd.print("P: ");
  lcd.setCursor(9,0);  lcd.print(dPeriod);
  lcd.setCursor(11,0); lcd.print("|");
  lcd.setCursor(12,0); lcd.print(aPeriod);
  lcd.setCursor(0,1);   lcd.print("T:");
  lcd.setCursor(2,1);   lcd.print(cTemp, 1);
  lcd.setCursor(8,1);  lcd.print(wDirection);
 lcd.setCursor(7,1);   lcd.print("D:");
  Serial.println("LCD updated");
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
 
void setup ()
{
  Serial.begin(115200);
  Serial.println("\n[surf Oracle running?]");
 
  if (ether.begin(sizeof Ethernet::buffer, mymac, 8) == 0)
  {
    Serial.println( "Error:Ethercard.begin");
    while(true);
  }
 Serial.println("\n[Here]");
  if (!ether.dhcpSetup())
  {
    Serial.println("DHCP failed");
    while(true);
  }
 
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip); 
  ether.printIp("DNS: ", ether.dnsip);
  
  //Write on the LCD Screen
  pinMode(red   ,OUTPUT); pinMode(green ,OUTPUT); pinMode(blue  ,OUTPUT);
  
  //For the Liquid Cristal
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, HIGH); // turn backlight on. Replace 'HIGH' with 'LOW' to turn it off.
  lcd.begin(20,4);              // columns, rows.  use 16,2 for a 16x2 LCD, etc.
  lcd.clear();   
  lcd.setCursor(0,0);  lcd.print("Initializing...");
  lcd.setCursor(0,1);  lcd.print("Reading waves info..");
  
 for (int i = 0; i < 5; i++) 
 {
    waveheight = i;
    updateColor();
    lcd.clear();   
    lcd.setCursor(0,0);  lcd.print("Light Color");
    if (i == 0) {lcd.setCursor(0,1);  lcd.print("test: Blue...");}
    if (i == 1) {lcd.setCursor(0,1);  lcd.print("test: Turquoise...");}
    if (i == 2) {lcd.setCursor(0,1);  lcd.print("test: Green...");}
    if (i == 3) {lcd.setCursor(0,1);  lcd.print("test: Orange...");}
    if (i == 4) {lcd.setCursor(0,1);  lcd.print("test: Red...");}
    delay(1000*3);
 }
 
  // Wait for link to become up - this speeds up the dnsLoopup in the current version of the Ethercard library
  while (!ether.isLinkUp())
  {
      ether.packetLoop(ether.packetReceive());
  }
  if (!ether.dnsLookup(website,false))
  {
    Serial.println("DNS failed. Unable to continue.");
    while (true);
  }
  ether.printIp("SRV: ", ether.hisip);
  Serial.println("Initializing...");
  lcd.clear();
 waveheight = 0; 
}
 
void loop ()
{
  ether.packetLoop(ether.packetReceive());

 if (millis() > timer)
  {

    Serial.println("\nSending request for page /data/latest_obs/46216.rss");
    foundPatternState=0;// Reset state machine
    Serial.println("Obtaining RSS info...");
    ether.persistTcpConnection(true);// enable persist, so that the callback is called for each received packet.
    ether.browseUrl(PSTR("/orbs/010/index.php"), "", website, browseUrlCallback);
    //This chage for each SurfOracle made
    //004:
    //005:
    //006: Denny Tyrrel, from Patagonia in Ventura
    //007: Sebastian Tapia
    //008: Don't know who have this one yet.
    //009: Adam Janke
    //010: Erendira Aceves
    updateLCD();
    updateColor();
    if (waveheight == 0)
    {
      timer = millis() + 5000;
      updateLCD();
      updateColor();
    }
    if (waveheight > 0)
    {
      timer = millis() + 1.8*1000000;// every 30 secs
    }
  }
  
}
 
