#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

//------------------------------------------
// RFID SPI Pins
//------------------------------------------

#define SS_PIN    10
#define RST_PIN   9

#define SPI_SCK   12
#define SPI_MISO  13
#define SPI_MOSI  11

MFRC522 rfid(SS_PIN, RST_PIN);


//------------------------------------------
// Wi-Fi
//------------------------------------------

const char* ssid = "Soumya";
const char* password = "12345678";


//------------------------------------------
// Google Apps Script URL
//------------------------------------------

const char* scriptURL =
"https://script.google.com/macros/s/AKfycbxT3CupS7zJHb8z0iVN2SDOFPhofDI_bVe71PAnlT18eWtTKzxhRQPrSZ5li-cwcATQ/exec";


//------------------------------------------
// Linen Database
//------------------------------------------

struct Linen
{
  String uid;
  String linenID;
  String type;
  String location;
  bool checkedIn;
};


Linen linens[] =
{
  {"F1EA9F53", "LINEN001", "Bed Sheet", "Coach A1", false},
  {"01939354", "LINEN002", "Blanket",   "Coach A1", false}
};

const int totalLinens =
  sizeof(linens) / sizeof(linens[0]);


//------------------------------------------
// Connect Wi-Fi
//------------------------------------------

void connectWiFi()
{
  Serial.print("Connecting to Wi-Fi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}


//------------------------------------------
// Send data to Google Sheets
//------------------------------------------

void sendToGoogleSheet(
  String uid,
  String linenID,
  String type,
  String location,
  String status)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wi-Fi disconnected!");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  // Encode spaces for URL
  type.replace(" ", "%20");
  location.replace(" ", "%20");
  status.replace(" ", "%20");

  String url = String(scriptURL);

  url += "?uid=" + uid;
  url += "&linenID=" + linenID;
  url += "&type=" + type;
  url += "&location=" + location;
  url += "&status=" + status;

  Serial.println();
  Serial.println("Sending data to Google Sheets...");
  Serial.println("URL:");
  Serial.println(url);

  http.begin(client, url);

  // Follow Google Apps Script redirect
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpResponseCode = http.GET();

  Serial.print("Google Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0)
  {
    String response = http.getString();

    Serial.print("Google Response: ");
    Serial.println(response);
  }
  else
  {
    Serial.print("HTTP Error: ");
    Serial.println(http.errorToString(httpResponseCode));
  }

  http.end();
}


//------------------------------------------
// SETUP
//------------------------------------------

void setup()
{
  Serial.begin(115200);

  //----------------------------------------
  // RFID
  //----------------------------------------

  SPI.begin(
    SPI_SCK,
    SPI_MISO,
    SPI_MOSI,
    SS_PIN
  );

  rfid.PCD_Init();


  //----------------------------------------
  // Wi-Fi
  //----------------------------------------

  connectWiFi();


  //----------------------------------------
  // System Message
  //----------------------------------------

  Serial.println();
  Serial.println("--------------------------------");
  Serial.println(" RAILWAY LINEN INVENTORY SYSTEM ");
  Serial.println("--------------------------------");
  Serial.println("Scan RFID Linen Tag...");
}


//------------------------------------------
// LOOP
//------------------------------------------

void loop()
{
  //----------------------------------------
  // Check RFID card
  //----------------------------------------

  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;


  //----------------------------------------
  // Read UID
  //----------------------------------------

  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++)
  {
    if (rfid.uid.uidByte[i] < 0x10)
      uid += "0";

    uid += String(
      rfid.uid.uidByte[i],
      HEX
    );
  }

  uid.toUpperCase();


  //----------------------------------------
  // Display UID
  //----------------------------------------

  Serial.println();
  Serial.println("=================================");

  Serial.print("RFID UID : ");
  Serial.println(uid);


  //----------------------------------------
  // Find Linen
  //----------------------------------------

  bool found = false;

  for (int i = 0; i < totalLinens; i++)
  {
    if (uid == linens[i].uid)
    {
      found = true;

      Serial.print("Linen ID : ");
      Serial.println(linens[i].linenID);

      Serial.print("Type     : ");
      Serial.println(linens[i].type);

      Serial.print("Location : ");
      Serial.println(linens[i].location);


      //------------------------------------
      // Check IN / CHECK OUT
      //------------------------------------

      String status;

      if (linens[i].checkedIn == false)
      {
        linens[i].checkedIn = true;

        status = "CHECKED IN";

        Serial.println("STATUS   : CHECKED IN");
      }
      else
      {
        linens[i].checkedIn = false;

        status = "CHECKED OUT";

        Serial.println("STATUS   : CHECKED OUT");
      }


      //------------------------------------
      // Send to Google Sheet
      //------------------------------------

      sendToGoogleSheet(
        uid,
        linens[i].linenID,
        linens[i].type,
        linens[i].location,
        status
      );

      break;
    }
  }


  //----------------------------------------
  // Unknown RFID
  //----------------------------------------

  if (!found)
  {
    Serial.println("STATUS   : UNKNOWN RFID TAG");
  }


  Serial.println("=================================");


  //----------------------------------------
  // Stop RFID
  //----------------------------------------

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(2000);
}