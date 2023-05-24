/*
Doom font
______             _                       __        __         __     _     _ _
| ___ \\           | |                     /  |      / /        /  |   | |   (_) |
| |_/ /_ _ ___ ___| | _____ _   _  __   __`| |     / /  __   __`| |   | |    _| |_ ___
|  __/ _` / __/ __| |/ / _ \\ | | | \\ \\ / / | |    / /   \\ \\ / / | |   | |   | | __/ _ \
| | | (_| \\__ \\__ \\   <  __/ |_| |  \\ V / _| |_  / /     \\ V / _| |_  | |___| | ||  __/
\\_|  \\__,_|___/___/_|\\_\\___|\\__, |   \\_/  \\___/ /_/       \\_/  \\___/  \\_____/_|\\__\\___|
                             __/ |
                            |___/

Project notes: Screen usage should be kept to a minimal as this program is for the base and light versions of Passkey v1
*/
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <Preferences.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <USB.h>
#include <USBHID.h>
#include <USBHIDKeyboard.h>
#include <USBCDC.h>

#if ARDUINO_USB_CDC_ON_BOOT
#define HWSerial Serial0
#define USBSerial Serial
#endif

TFT_eSPI tft = TFT_eSPI();
Preferences pref = Preferences();
USBHID hid = USBHID();
USBHIDKeyboard Keyboard = USBHIDKeyboard();
USBCDC cdc = USBCDC();

String ArrayResult[100];
int ArrayResultSize = 0;
void splitString(String ogString, String delimiter = ",")
{
  int arraySize = ogString.length(); // Number of elements in the array
  String stringArray[arraySize];     // Declare the String array with a fixed size
  int index = 0;                     // Index for the String array

  // Split the inputString with the delimiter ","
  while (ogString.length() > 0 && index < arraySize)
  {
    int commaIndex = ogString.indexOf(delimiter);
    if (commaIndex >= 0)
    {
      stringArray[index] = ogString.substring(0, commaIndex);
      ogString = ogString.substring(commaIndex + 1);
    }
    else
    {
      stringArray[index] = ogString;
      ogString = "";
    }
    index++;
  }
  for (int i = 0; i < arraySize; i++)
  {
    ArrayResult[i] = stringArray[i];
  }
  ArrayResultSize = arraySize;
}

void setup()
{
  Serial.begin(115200);
  HWSerial.begin(115200);
  cdc.begin(115200);
  delay(1000);
  pref.begin("lockdown");
  tft.begin();
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.println("Starting fs...");
  if (!SPIFFS.begin(true))
  {
    tft.setTextColor(TFT_RED);
    tft.println("FATAL: SPIFFS Failed to Start");
    return;
  }
  tft.println("Checking Settings & Preferencs files...");
  
  if (!SPIFFS.exists("/settings.json"))
  {
    tft.println("Creating Settings File");
    fs::File settingsFile = SPIFFS.open("/settings.json", "w", true);
    settingsFile.close();
  }
  if (!SPIFFS.exists("/passwords.json"))
  {
    tft.println("Creating Passwords File");
    fs::File passwordsFile = SPIFFS.open("/passwords.json", "w", true);
    passwordsFile.close();
  }
  tft.println("Testing Serial before keyboard init...");
  Serial.println("Serial before keyboard init\nConnect to COM9/alt. port");
  tft.println("Initializing HID...");
  USB.begin();
  hid.begin();
  Keyboard.begin();
  tft.println("Testing Serial after keyboard init...");
  cdc.println("Serial after keyboard init");
  Serial.println("Serial after keyboard init");
  tft.println("Turning off Backlight");
  if (digitalRead(14) == LOW)
  {
    pref.putBool("lockdown", false);
    pref.putString("masterpasswd", "t6R35bQBxSK85PLR");
    delay(1000);
    Serial.println("Deleting Credentials File...");
    cdc.println("Deleting Credentials File...");
    SPIFFS.remove("/passwords.json");
    Serial.println("Loading Fake Credentials...");
    cdc.println("Loading Fake Credentials...");
    fs::File file = SPIFFS.open("/passwords.json", "w", true);
    StaticJsonDocument<4000> doc2;
    doc2["google.com:hyper_creeper"] = "password";
    String output;
    serializeJson(doc2, output);
    file.print(output);
    file.close();
    tft.println("Placed Fake credentials\nRequest: google.com:hyper_creeper\nPassword: password\n\nMASTER PASSWORD: t6R35bQBxSK85PLR");
    Serial.println("Placed Fake credentials\nRequest: google.com:hyper_creeper\nPassword: password\n\nMASTER PASSWORD: t6R35bQBxSK85PLR");
    cdc.println("Placed Fake credentials\nRequest: google.com:hyper_creeper\nPassword: password\n\nMASTER PASSWORD: t6R35bQBxSK85PLR");
  }
  delay(100);
  cdc.println("REQUESTING MASTER PASSWORD");
  String masterpwdreq = "";
  tft.println("Waiting for Master Password");
  while (!pref.getBool("lockdown", false))
  {
    if (cdc.available() > 0)
    {
      masterpwdreq = cdc.readString();
      masterpwdreq.replace("\r\n", "");
      cdc.println("Checking: " + String(masterpwdreq));
      break;
    }
  }
  if (pref.getBool("lockdown", false) || pref.getString("masterpasswd", "none") != masterpwdreq)
  {
    digitalWrite(TFT_BL, HIGH);
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.drawCentreString("LOCKDOWN", tft.width() / 2, tft.height() / 2, 1);
    tft.drawCentreString("MODE", tft.width() / 2, tft.height() / 2 + 20, 1);
    while (true)
    {
    }
  }
#ifdef PASSKEY_V1_LITE
  cdc.println("______             _                       __     _     _ _       \n | ___ \\           | |                     /  |   | |   (_) |      \n | |_/ /_ _ ___ ___| | _____ _   _  __   __`| |   | |    _| |_ ___ \n |  __/ _` / __/ __| |/ / _ \\ | | | \\ \\ / / | |   | |   | | __/ _ \\n | | | (_| \\__ \\__ \\   <  __/ |_| |  \\ V / _| |_  | |___| | ||  __/\n \\_|  \\__,_|___/___/_|\\_\\___|\\__, |   \\_/  \\___/  \\_____/_|\\__\\___|\n __/ |                                \n |___/                                 \n");
#else
  cdc.println("______             _                       __  \n | ___ \\           | |                     /  | \n | |_/ /_ _ ___ ___| | _____ _   _  __   __`| | \n |  __/ _` / __/ __| |/ / _ \\ | | | \\ \\ / / | | \n | | | (_| \\__ \\__ \\   <  __/ |_| |  \\ V / _| |_\n \\_|  \\__,_|___/___/_|\\_\\___|\\__, |   \\_/  \\___/\n __/ |             \n |___/              \n");
#endif
  digitalWrite(TFT_BL, LOW);
}
String result = "";
void loop()
{
  // put your main code here, to run repeatedly:
  if (cdc.available() > 0)
  {
    result = cdc.readStringUntil('\n');
    result.replace("\r", "");
    result.replace("\n", "");
    if (result.substring(0, 4) == "get ")
    {
      if(result.indexOf("get all") > -1) {
        StaticJsonDocument<200> doc;
        fs::File file = SPIFFS.open("/passwords.json", "r", false);
        String docData = file.readString();
        deserializeJson(doc, docData);
        String data = "";
        for (JsonPair kv : doc.as<JsonObject>())
        {
          String temp = kv.key().c_str();
          data += String("," + temp);
        }
        data.remove(0, 1);
        cdc.println(data);
        file.close();
      }
      else {
        result.replace("get ", "");
        String SiteName = result.substring(0, result.indexOf(":"));
        String AccountName = result.substring(result.indexOf(":") + 1, result.indexOf(" "));
        cdc.println(String(SiteName + ":" + AccountName));
        StaticJsonDocument<200> doc;
        fs::File file = SPIFFS.open("/passwords.json", "r", false);
        deserializeJson(doc, file.readString());
        const char *data = doc[String(SiteName + ":" + AccountName)];
        cdc.println(data);
        file.close();
      }
    }
    else if (result.substring(0, 4) == "set ")
    {
      result.replace("set ", "");
      String SiteName = result.substring(0, result.indexOf(":"));
      String AccountName = result.substring(result.indexOf(":") + 1, result.indexOf(" "));
      StaticJsonDocument<4000> doc2;
      cdc.println(String(SiteName + ":" + AccountName));
      fs::File file = SPIFFS.open("/passwords.json", "r", true);
      deserializeJson(doc2, file.readString());
      file.close();
      fs::File fileWrite = SPIFFS.open("/passwords.json", "w", true);
      doc2[String(SiteName + ":" + AccountName)] = result.substring(result.indexOf(" ") + 1);
      String output;
      serializeJson(doc2, output);
      fileWrite.print(output);
      cdc.println("Set Successfully!");
      fileWrite.close();
    }
    else if (result == "lockdown")
    {
      pref.putBool("lockdown", true);
      digitalWrite(TFT_BL, HIGH);
      tft.fillScreen(TFT_RED);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(2);
      tft.drawCentreString("LOCKDOWN", tft.width() / 2, tft.height() / 2, 1);
      tft.drawCentreString("MODE", tft.width() / 2, tft.height() / 2 + 20, 1);
      while (true)
      {
      }
    }
    else if (result == "unlockdown")
    {
      while (cdc.available() > 0)
      {
        if (pref.getString("masterpasswd", "t6R35bQBxSK85PLR") == cdc.readStringUntil('\n'))
        {
          pref.putBool("lockdown", false);
          digitalWrite(TFT_BL, LOW);
          tft.fillScreen(TFT_BLACK);
          ESP.restart();
        }
      }
    }
    else if (result.substring(0, 16) == "changemasterpwd ")
    {
      delay(200);
      cdc.println("REQUESTING MASTER PASSWORD");
      while (true)
      {
        if (cdc.available() > 0)
        {
          String masterpwdreq = cdc.readString();
          masterpwdreq.replace("\r\n", "");
          if (masterpwdreq == pref.getString("masterpasswd", "t6R35bQBxSK85PLR"))
          {
            pref.putString("masterpasswd", result.substring(result.indexOf(" ") + 1));
            cdc.println("Master Password Successfully set!");
            break;
          }
        }
      }
    }
    else if (result.substring(0, 5) == "type ")
    {
      result.replace("type ", "");
      delay(2000);
      Keyboard.print(result);
    }
    else if (result.substring(0, 9) == "settings ")
    {
      result.replace("settings ", "");
      if (result.substring(0, result.indexOf(" ")) == "setall")
      {
        result.replace("setall ", "");
        fs::File file = SPIFFS.open("/settings.json", "w", true);
        file.print(result);
        file.close();
      }
      else if (result.substring(0, result.indexOf(" ")) == "set ")
      {
        result.replace("set ", "");
        fs::File file = SPIFFS.open("/settings.json", "r", false);
        StaticJsonDocument<4000> doc2;
        deserializeJson(doc2, file.readString());
        file.close();
        fs::File writeFile = SPIFFS.open("/settings.json", "w", false);
        doc2[String(result.substring(0, result.indexOf(" ")))] = result.substring(result.indexOf(" ") + 1);
        String output;
        serializeJson(doc2, output);
        writeFile.print(output);
        writeFile.close();
      }
    }
    else if (result == "help")
    {
      cdc.println("get <sitename>:<accountname> | Get <sitename>:<accountname>'s password\nget all | Get all usernames\nset <sitename>:<accountname> <password> | Set a password for a username\nlockdown | Put device in lockdown mode\ntype <text> | Force text to be typed as keyboard\n settings setall <json> | Overwrite all settings with new json\nsettings set <settings <value> | Set a setting to corresponding value\nchangemasterpwd password | Change Passkey's Master Password");
    }
    else
    {
      cdc.println("Error: Invalid Command: " + result + "|");
    }
  }
}