#line 1 "/home/jens/Dropbox/ESP8266/followercounter-xl/followercounter-xl.ino"
#line 1 "/home/jens/Dropbox/ESP8266/followercounter-xl/followercounter-xl.ino"


#include <FS.h> //this needs to be first, or it all crashes and burns...
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "InstagramStats.h"      // InstagramStats              https://github.com/witnessmenow/arduino-instagram-stats
#include "JsonStreamingParser.h" // Json Streaming Parser

#include <ESP8266HTTPClient.h> // Web Download
#include <ESP8266httpUpdate.h> // Web Updater

#include <ArduinoJson.h> // ArduinoJSON                 https://github.com/bblanchon/ArduinoJson

#include <DNSServer.h>        // - Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> // - Local WebServer used to serve the configuration portal
#include <WiFiManager.h>      // WifiManager

#include <NTPClient.h>
#include <time.h>

#include <Arduino.h>
//#include <U8g2lib.h>
#include <SPI.h>

#define PGM_READ_UNALIGNED 0 

#include <ESPStringTemplate.h>

#include "index.h"

#include <U8g2_for_Adafruit_GFX.h>

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif

#define VERSION "1.9rc5"
#define ROTATE 90
#define USE_SERIAL Serial

// for NodeMCU 1.0
#define DIN_PIN 15   // D8
#define TOGGLE_PIN 0 // D3


static const uint16_t PROGMEM
    // These bitmaps were written for a backend that only supported
    // 4 bits per color with Blue/Green/Red ordering while neomatrix
    // uses native 565 color mapping as RGB.  
    // I'm leaving the arrays as is because it's easier to read
    // which color is what when separated on a 4bit boundary
    // The demo code will modify the arrays at runtime to be compatible
    // with the neomatrix color ordering and bit depth.
    RGB_bmp[][64] = {
      
      // 10: multicolor smiley face
      { 
        0x4AB9, 0x5299, 0x5A98, 0x6A57, 0x7A17, 0x89B6, 0x9177, 0x8997, 
        0x6996, 0x7174, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xA934, 0xA955, 
        0x89B3, 0xFFFF, 0xB171, 0xB94F, 0xC12F, 0xFFFF, 0xFFFF, 0xF01E, 
        0xA971, 0xFFFF, 0xC94D, 0xC92D, 0xD12E, 0xD12D, 0xFFFF, 0xC931, 
        0xCA6B, 0xFFFF, 0xE267, 0xE248, 0xE1AA, 0xD90C, 0xFFFF, 0xD150, 
        0xE366, 0xFFFF, 0xF3A3, 0xF303, 0xEA85, 0xE1E9, 0xFFFF, 0xD12F, 
        0xF464, 0xF4E4, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xE16A, 0xD12E, 
        0xF5A9, 0xFE4B, 0xFE6C, 0xFE2B, 0xF569, 0xF3E7, 0xE20A, 0xD14F, 
       },
};

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, DIN_PIN,
                                               NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
                                               NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);

U8G2_FOR_ADAFRUIT_GFX u8g2;

const long interval = 3000 * 1000; // alle 60 Minuten prüfen
unsigned long previousMillis = millis() - 2980 * 1000;
unsigned long lastPressed = millis();

WiFiClientSecure client;

InstagramStats instaStats(client);
ESP8266WebServer server(80);

char time_value[20];

int textsize = 0;

int follower;
int modules = 4;

int mode = 1;
int modetoggle = 0;

// Variables will change:
int buttonPushCounter = 0; // counter for the number of button presses
int buttonState = 1;       // current state of the button
int lastButtonState = 1;   // previous state of the button

const uint8_t DotMatrixCondensed[1436] U8G2_FONT_SECTION("DotMatrixCondensed") =
  "\276\0\2\2\2\3\3\4\4\3\6\0\377\5\377\5\0\0\352\1\330\5\177 \5\340\315\0!\6\265\310"
  "\254\0\42\6\213\313$\25#\10\227\310\244\241\206\12$\10\227\310\215\70b\2%\10\227\310d\324F\1"
  "&\10\227\310(\65R\22'\5\251\313\10(\6\266\310\251\62)\10\226\310\304\224\24\0*\6\217\312\244"
  "\16+\7\217\311\245\225\0,\6\212\310)\0-\5\207\312\14.\5\245\310\4/\7\227\310Ve\4\60"
  "\7\227\310-k\1\61\6\226\310\255\6\62\10\227\310h\220\312\1\63\11\227\310h\220\62X\0\64\10\227"
  "\310$\65b\1\65\10\227\310\214\250\301\2\66\10\227\310\315\221F\0\67\10\227\310\314TF\0\70\10\227"
  "\310\214\64\324\10\71\10\227\310\214\64\342\2:\6\255\311\244\0;\7\222\310e\240\0<\10\227\310\246\32"
  "d\20=\6\217\311l\60>\11\227\310d\220A*\1\77\10\227\310\314\224a\2@\10\227\310UC\3"
  "\1A\10\227\310UC\251\0B\10\227\310\250\264\322\2C\7\227\310\315\32\10D\10\227\310\250d-\0"
  "E\10\227\310\214\70\342\0F\10\227\310\214\70b\4G\10\227\310\315\221\222\0H\10\227\310$\65\224\12"
  "I\7\227\310\254X\15J\7\227\310\226\252\2K\10\227\310$\265\222\12L\7\227\310\304\346\0M\10\227"
  "\310\244\61\224\12N\10\227\310\244q\250\0O\7\227\310UV\5P\10\227\310\250\264b\4Q\10\227\310"
  "Uj$\1R\10\227\310\250\64V\1S\10\227\310m\220\301\2T\7\227\310\254\330\2U\7\227\310$"
  "W\22V\10\227\310$\253L\0W\10\227\310$\65\206\12X\10\227\310$\325R\1Y\10\227\310$U"
  "V\0Z\7\227\310\314T\16[\7\227\310\214X\16\134\10\217\311d\220A\0]\7\227\310\314r\4^"
  "\5\213\313\65_\5\207\310\14`\6\212\313\304\0a\7\223\310\310\65\2b\10\227\310D\225\324\2c\7"
  "\223\310\315\14\4d\10\227\310\246\245\222\0e\6\223\310\235\2f\10\227\310\246\264b\2g\10\227\307\35"
  "\61%\0h\10\227\310D\225\254\0i\6\265\310\244\1j\10\233\307f\30U\5k\10\227\310\304\264T"
  "\1l\7\227\310\310\326\0m\7\223\310<R\0n\7\223\310\250d\5o\7\223\310U\252\2p\10\227"
  "\307\250\244V\4q\10\227\307-\225d\0r\6\223\310\315\22s\10\223\310\215\70\22\0t\10\227\310\245"
  "\25\243\0u\7\223\310$+\11v\10\223\310$\65R\2w\7\223\310\244q\4x\7\223\310\244\62\25"
  "y\11\227\307$\225dJ\0z\7\223\310\254\221\6{\10\227\310\251\32D\1|\6\265\310(\1}\11"
  "\227\310\310\14RR\0~\6\213\313\215\4\241\6\265\310\244\1\242\10\227\310\245\21W\2\243\10\227\310\251"
  "\264\322\0\244\7\227\310\244j\65\245\10\227\310$U\255\4\246\6\265\310(\1\247\7\227\310\251.\5\250"
  "\6\207\314\244\0\251\6\217\312m \252\7\227\310\35\31\14\253\6\216\312\311\0\254\6\213\312\314\0\255\5"
  "\206\312\10\256\6\217\312(U\257\5\207\314\14\260\6\217\312u\1\261\10\227\310\245\225\321\0\262\6\217\312"
  "\310(\263\6\217\312\254!\264\6\252\313)\0\265\10\227\310$kE\0\266\7\227\310\255\344\0\267\5\217"
  "\311<\270\7\217\310e\260\0\271\5\255\312\14\272\7\227\310u\243\1\273\6\256\312D\5\274\10\227\310\304"
  "L\310\0\275\7\227\310\304\14\15\276\10\227\310(\15e\0\277\10\227\310e\230\342\0\300\11\227\310e\220"
  "\322H\1\301\10\227\310\325 \215\24\302\10\227\310l\224F\12\303\10\227\310\215\230F\12\304\10\227\310\244"
  "\326P\1\305\10\227\310(\225\206\12\306\10\227\310\215\64\324\0\307\11\233\307\315\14dJ\0\310\7\227\310"
  "ep\15\311\7\227\310\225C\15\312\10\227\310l\60\324\0\313\10\227\310\244\14\206\32\314\10\227\310e\60"
  "R\32\315\7\227\310\225+\15\316\10\227\310l\260\322\0\317\10\227\310\244\14V\32\320\10\227\310\250\64\324"
  "\2\321\10\227\310\310\65T\0\322\10\227\310e\60\324\10\323\10\227\310\225#\215\0\324\10\227\310l\60\322"
  "\10\325\10\227\310\310\261F\0\326\11\227\310\244\14F\32\1\327\6\217\311\244\16\330\7\227\310\35j\1\331"
  "\11\227\310d\220\222\32\1\332\10\227\310\246J\215\0\333\10\227\310l\220\324\10\334\11\227\310\244\14\222\32"
  "\1\335\10\227\310\246j\244\4\336\10\227\310\304\221\206\4\337\10\233\307]iE\0\340\10\227\310e\220\216"
  "\0\341\10\227\310\325`\215\0\342\7\227\310lt\4\343\10\227\310\215\270F\0\344\10\227\310\244\214\216\0"
  "\345\7\227\310YG\0\346\7\223\310\215#\1\347\10\227\307m S\2\350\10\227\310e\220\206\22\351\10"
  "\227\310\325`(\1\352\10\227\310l\64\224\0\353\10\227\310\244\214\206\22\354\7\266\310DU\1\355\6\226"
  "\310\311T\356\10\227\310l\24+\0\357\10\227\310\244\214b\5\360\10\227\310\215\270\222\0\361\10\227\310\310"
  "\221\222\12\362\10\227\310e\220\272\0\363\10\227\310\325 \265\0\364\7\227\310l\324\5\365\7\227\310H\325"
  "\5\366\10\227\310\244\214\272\0\367\10\227\310e\264Q\2\370\7\223\310\215\265\0\371\11\227\310d\220\222J"
  "\2\372\10\227\310\246J%\1\373\10\227\310l\220T\22\374\11\227\310\244\14\222J\2\375\10\233\307\246\226"
  "L\11\376\10\227\307D\225V\4\377\12\233\307\244\14R\222)\1\0\0\0\4\377\377\0";

//define your default values here, if there are different values in config.json, they are overwritten.
char instagramName[40];
char matrixIntensity[5];
char maxModules[5];
char htmlBuffer[4096];

// =======================================================================

//flag for saving data
bool shouldSaveConfig = true;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void handleRoot()
{

  ESPStringTemplate webpage(htmlBuffer, sizeof(htmlBuffer));

  TokenStringPair pair[1];
  pair[0].setPair("%INSTAGRAM%", instagramName);

  webpage.add_P(_PAGE_HEAD);
  webpage.add_P(_PAGE_START);
  webpage.add_P(_PAGE_ACTIONS);
  webpage.add_P(_PAGE_CONFIG_NAME, pair, 1);

  switch (mode)
  {
  case 1:
    webpage.add_P(_PAGE_CONFIG_MODE1);
    break;

  case 2:
    webpage.add_P(_PAGE_CONFIG_MODE2);
    break;

  case 3:

    webpage.add_P(_PAGE_CONFIG_MODE3);
    break;

    break;

  default:
    webpage.add_P(_PAGE_CONFIG_MODE1);
    break;
  }

  TokenStringPair intensityPair[1];

  intensityPair[0].setPair("%INTENSITY%", matrixIntensity);
  webpage.add_P(_PAGE_CONFIG_INTENSITY, intensityPair, 1);
  webpage.add_P(_PAGE_FOOTER);

  server.send(200, "text/html", htmlBuffer);
}

void redirectBack()
{
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void getConfig()
{

  // instagramName
  String instagramNameString = server.arg("instagramname");
  instagramNameString.toCharArray(instagramName, 40);

  // mode
  String modeString = server.arg("mode");
  mode = modeString.toInt();

  // Intensity
  String intensityString = server.arg("intensity");
  String matrixIntensityString = intensityString;
  matrixIntensityString.toCharArray(matrixIntensity, 40);

  matrix.setBrightness(matrixIntensityString.toInt());
  matrix.show();

  saveConfig();

  redirectBack();
}

void getReset()
{
  redirectBack();
  restartX();
}

void getUpdate()
{
  redirectBack();
  updateFirmware();
}

void getFormat()
{
  redirectBack();
  infoReset();
}

void clearBuffer()
{
  //matrix->clear();
}

void printString(int x, int y, String output, int font)
{

  u8g2.setForegroundColor(matrix.Color(0,135,255));

  if (font == 1)  {

    u8g2.setFont(u8g2_font_finderskeepers_tf);
  
  } else {

    u8g2.setFont(DotMatrixCondensed);
  }

  


  u8g2.setCursor(x, y);
  u8g2.print(output.c_str());
 
  matrix.show();
}


void fixdrawRGBBitmap() {
  

  
  matrix.drawRGBBitmap(0, 0, RGB_bmp[0], 8, 8);
}

void printLogo()  {

  fixdrawRGBBitmap();

  matrix.show();
}


void printPixel(int x, int y)
{

  //u8g2.drawPixel(31, 0);
  //u8g2.sendBuffer();
}

void printHline(int a, int b, int c)
{
  //u8g2.drawHLine(a, b, c);
}
void printVline(int a, int b, int c, int d)
{
  //uu8g2.drawLine(a, b, c, d);
}

void setup()
{

  // Serial debugging
  Serial.begin(115200);

  // Required for instagram api
  client.setInsecure();

  // Set Reset-Pin to Input Mode
  pinMode(TOGGLE_PIN, INPUT);

  if (SPIFFS.begin())
  {

    if (SPIFFS.exists("/config.json"))
    {
      //file exists, reading and loading

      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);
        deserializeJson(json, buf.get());
        serializeJson(json, Serial);

        strcpy(instagramName, json["instagramName"]);
        strcpy(maxModules, json["maxModules"]);

        JsonVariant jsonMatrixIntensity = json["matrixIntensity"];
        if (!jsonMatrixIntensity.isNull())
        {
          strcpy(matrixIntensity, json["matrixIntensity"]);
        }

        JsonVariant jsonMode = json["mode"];
        if (!jsonMode.isNull())
        {
          mode = jsonMode.as<int>();
        }
        else
        {
        }
      }
    }
    else
    {
    }
  }
  else
  {
    Serial.println("failed to mount FS");
  }
  //end read

  WiFiManager wifiManager;

  // Requesting Instagram and Intensity for Display
  WiFiManagerParameter custom_instagram("Instagram", "Instagram", instagramName, 40);
  WiFiManagerParameter custom_intensity("Helligkeit", "Helligkeit 0-254", matrixIntensity, 5);
  WiFiManagerParameter custom_modules("Elemente", "Anzahl Elemente 4-8", maxModules, 5);

  // Add params to wifiManager
  wifiManager.addParameter(&custom_instagram);
  wifiManager.addParameter(&custom_intensity);
  wifiManager.addParameter(&custom_modules);

  // Warte damit das Display initialisiert werden kannu
  delay(1000);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(10);
  matrix.setTextColor(matrix.Color(255, 0, 0));

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 0); // https://github.com/nayarsystems/posix_tz_db


  u8g2.begin(matrix);                 // connect u8g2 procedures to Adafruit GFX


  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.autoConnect("FollowerCounter");

  server.on("/", handleRoot);
  server.on("/format", getFormat);
  server.on("/update", getUpdate);
  server.on("/reset", getReset);
  server.on("/config", getConfig);

  server.begin();

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //read updated parametersu
  strcpy(instagramName, custom_instagram.getValue());
  strcpy(matrixIntensity, custom_intensity.getValue());
  strcpy(maxModules, custom_modules.getValue());

  // modules = String(maxModules).toInt();

  String matrixIntensityString = matrixIntensity;

  matrix.setBrightness(matrixIntensityString.toInt());

 

  //save the custom parameters to FS
  if (shouldSaveConfig)
  {

    saveConfig();

    //end save
  }

  printString(0, 7, "Starte", 1);



}

void saveConfig()
{

  DynamicJsonDocument json(1024);

  json["instagramName"] = instagramName;
  json["matrixIntensity"] = matrixIntensity;
  json["maxModules"] = maxModules;
  json["mode"] = mode;

  File configFile = SPIFFS.open("/config.json", "w");

  if (!configFile)
  {
    Serial.println("failed to open config file for writing");
  }

  serializeJson(json, Serial);
  serializeJson(json, configFile);
}

void infoWlan()
{

  if (WiFi.status() == WL_CONNECTED)
  {

    // WLAN Ok
    printString(0, 7, "WIFI OK", 1);
  }
  else
  {

    // WLAN Error
    printString(0, 7, "WIFI Error", 1);
  }
}

void infoIP()
{
  String localIP = WiFi.localIP().toString();

  printString(0, 7, localIP, 1);
  delay(1000);
  printString(0, 7, localIP.substring(8), 1);
}

void infoVersion()
{

  char versionString[8];
  sprintf(versionString, "Ver. %s", VERSION);
  printString(0, 7, versionString, 1);
}

void infoReset()
{

  Serial.println("Format System");

  printString(0, 7, "Format", 1);

  // Reset Wifi-Setting
  WiFiManager wifiManager;
  wifiManager.resetSettings();

  // Format Flash
  SPIFFS.format();

  // Restart
  ESP.reset();
}

void restartX()
{

  printString(0, 7, "Restarte…", 1);
  ESP.reset();
}


void update_started()
{

  printString(0, 7, "Update …", 1);
  USE_SERIAL.println("CALLBACK:  HTTP update process started");
}

void update_finished()
{

  printString(0, 7, "Done", 1);
  USE_SERIAL.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total)
{
  char progressString[10];
  float percent = ((float)cur / (float)total) * 100;
  sprintf(progressString, " %s", String(percent).c_str());
  printString(0, 7, progressString, 1);
  USE_SERIAL.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err)
{
  char errorString[8];
  sprintf(errorString, "Err %d", err);
  printString(0, 7, errorString, 1);
  USE_SERIAL.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void updateFirmware()
{

  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

  // Add optional callback notifiers
  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);

  t_httpUpdate_return ret = ESPhttpUpdate.update(client, "https://counter.buuild.it/static/themes/counter/followercounter-xl.ino.bin");

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    USE_SERIAL.println("HTTP_UPDATE_OK");
    break;
  }
}

//
void loop()
{

  server.handleClient();

  buttonState = digitalRead(TOGGLE_PIN);
  unsigned long currentMillis = millis();

  if (currentMillis % 2000 == 0)
  {

    //helloFullScreenPartialMode(timeString);
  }

  if (buttonState != lastButtonState && currentMillis > lastPressed + 50)
  {

    // if the state has changed, increment the counter
    if (buttonState == LOW)
    {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter++;
      lastPressed = currentMillis;

      Serial.println("push");

      printPixel(31, 0);

      Serial.println(buttonPushCounter);
    }
    else
    {
      // if the current state is LOW then the button went from on to off:
      Serial.println("off");
    }
  }

  // Warte 1sec nach dem letzten Tastendruck
  if (currentMillis > lastPressed + 1000)
  {

    if (buttonPushCounter > 0)
    {

      Serial.print("number of button pushes: ");
      Serial.println(buttonPushCounter);

      switch (buttonPushCounter)
      {

      case 1:
        // Einmal gedrückt / FollowerCounter-Modus
        mode = 1;
        break;

      case 2:
        // Zweimal gedrückt / Uhrzeit-Modus
        mode = 2;
        break;

      case 3:
        // Dreimal gedrückt / Wechselmodus
        mode = 3;
        break;

      case 4:
        infoWlan();
        break;

      case 5:
        infoIP();
        break;

      case 6:
        infoVersion();
        break;

      case 7:
        updateFirmware();
        break;

      case 8:
        restartX();
        break;

      case 10:
        infoReset();
        break;

      default:

        printString(0, 7, "Too many", 1);
        break;
      }
    }

    buttonPushCounter = 0;
  }

  // save the current state as the last state, for next time through the loop
  lastButtonState = buttonState;

  // Update follower count
  if (currentMillis - previousMillis >= interval)
  {

    previousMillis = currentMillis;
    Serial.println(instagramName);

    InstagramUserStats response = instaStats.getUserStats(instagramName);
    Serial.print("Number of followers: ");
    Serial.println(response.followedByCount);

    int currentCount = response.followedByCount;

    if (currentCount > 0)
    {
      follower = currentCount;
    }
  }

  if (currentMillis % 10000 == 0)
  {

    switch (mode)
    {
    case 1:
      /* code */
      printCurrentFollower();
      break;

    case 2:
      /* code */
      printTime();
      break;

    case 3:

      if (modetoggle == 1)
      {

        modetoggle = 0;
        printTime();
      }
      else
      {

        printCurrentFollower();
        modetoggle = 1;
      }

      break;

    default:
      break;
    }
  }
}

void printTime()
{

  
  time_t now = time(nullptr);
  String time = String(ctime(&now));
  time.trim();
  time.substring(11, 16).toCharArray(time_value, 10);

  matrix.fillScreen(0);
  printString(6, 6, time_value, 2);
}

void printCurrentFollower()
{

  String instacount = String(follower);

  char copy[instacount.length() + 1];
  instacount.toCharArray(copy, instacount.length() + 1);

  if (follower > 0)
  {

    int modules = String(maxModules).toInt();

    

      clearBuffer();
      matrix.fillScreen(0);
      printLogo();
      printString(10, 6, copy, 2);
    
  }
}

