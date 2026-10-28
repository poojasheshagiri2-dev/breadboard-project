#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =====================================================
// RFID CONNECTIONS - ESP32-S3
// =====================================================

#define SS_PIN    10
#define RST_PIN   9

#define SPI_SCK   12
#define SPI_MISO  13
#define SPI_MOSI  11

MFRC522 rfid(SS_PIN, RST_PIN);


// =====================================================
// LCD CONNECTIONS
// =====================================================

#define LCD_SDA 8
#define LCD_SCL 7

LiquidCrystal_I2C lcd(0x27, 16, 2);


// =====================================================
// LED CONNECTIONS
// =====================================================

#define GREEN_LED  4
#define YELLOW_LED 5
#define RED_LED    6


// =====================================================
// WIFI DETAILS
// =====================================================

const char* ssid = "Airtel_ling_3806";
const char* password = "air35458";


// =====================================================
// GOOGLE APPS SCRIPT WEB APP URL
// =====================================================

const char* scriptURL =
"https://script.google.com/macros/s/AKfycby96StIIzKN-BBJ3Z0uUrojQuaeAqXf_sWtHj3v4Sc4KUoeoxceClMTkZzJTuw-Bx8P/exec";


// =====================================================
// LINEN DATABASE
// =====================================================

struct Linen
{
  String uid;
  String linenID;
  String type;
  String location;
  bool checkedIn;
};


// -----------------------------------------------------
// Add your RFID tags here
// -----------------------------------------------------

Linen linens[] =
{
  {"F1EA9F53", "LINEN001", "Bed Sheet", "Coach A1", false},

  {"01939354", "LINEN002", "Blanket", "Coach A1", false}
};


const int totalLinens =
  sizeof(linens) / sizeof(linens[0]); 


// =====================================================
// LED FUNCTIONS
// =====================================================

void allLEDOff()
{
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
}


void greenLED()
{
  allLEDOff();
  digitalWrite(GREEN_LED, HIGH);
}


void yellowLED()
{
  allLEDOff();
  digitalWrite(YELLOW_LED, HIGH);
}


void redLED()
{
  allLEDOff();
  digitalWrite(RED_LED, HIGH);
}


// =====================================================
// LCD FUNCTIONS
// =====================================================

void showReadyScreen()
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("RAILWAY LINEN");

  lcd.setCursor(0, 1);
  lcd.print("SCAN RFID...");
}


void showAvailable(String linenID)
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(linenID);

  lcd.setCursor(0, 1);
  lcd.print("AVAILABLE");

  greenLED();
}


void showMissing(String linenID)
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(linenID);

  lcd.setCursor(0, 1);
  lcd.print("MISSING");

  redLED();
}


void showUnknown()
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("UNKNOWN RFID");

  lcd.setCursor(0, 1);
  lcd.print("ITEM NOT FOUND");

  yellowLED();
}


// =====================================================
// WIFI CONNECTION
// =====================================================

void connectWiFi()
{
  Serial.println();
  Serial.print("Connecting to Wi-Fi");

  WiFi.begin(ssid, password);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30)
  {
    delay(500);

    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Wi-Fi Connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Wi-Fi connection failed!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WIFI ERROR");

    lcd.setCursor(0, 1);
    lcd.print("CHECK CONNECTION");

    delay(3000);
  }
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

// =====================================================
// DISPLAY LINEN INFORMATION
// =====================================================

void displayLinenInfo(int index)
{
  Serial.println();

  Serial.println("--------------------------------");

  Serial.print("RFID UID : ");
  Serial.println(linens[index].uid);

  Serial.print("Linen ID : ");
  Serial.println(linens[index].linenID);

  Serial.print("Type     : ");
  Serial.println(linens[index].type);

  Serial.print("Location : ");
  Serial.println(linens[index].location);

  Serial.print("Status   : ");

  if (linens[index].checkedIn)
  {
    Serial.println("AVAILABLE");
  }
  else
  {
    Serial.println("MISSING");
  }

  Serial.println("--------------------------------");
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
  // ---------------------------------------------------
  // Serial Monitor
  // ---------------------------------------------------

  Serial.begin(115200);

  delay(1000);


  // ---------------------------------------------------
  // LED setup
  // ---------------------------------------------------

  pinMode(GREEN_LED, OUTPUT);

  pinMode(YELLOW_LED, OUTPUT);

  pinMode(RED_LED, OUTPUT);

  allLEDOff();


  // ---------------------------------------------------
  // LCD setup
  // ---------------------------------------------------

  Wire.begin(
    LCD_SDA,
    LCD_SCL
  );


  lcd.init();

  lcd.backlight();


  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("RAILWAY LINEN");

  lcd.setCursor(0, 1);

  lcd.print("STARTING...");


  delay(2000);


  // ---------------------------------------------------
  // RFID SPI
  // ---------------------------------------------------

  SPI.begin(
    SPI_SCK,
    SPI_MISO,
    SPI_MOSI,
    SS_PIN
  );


  rfid.PCD_Init();


  Serial.println();

  Serial.println("RFID Reader Initialized");


  // ---------------------------------------------------
  // Wi-Fi
  // ---------------------------------------------------

  connectWiFi();


  // ---------------------------------------------------
  // Ready
  // ---------------------------------------------------

  allLEDOff();

  showReadyScreen();


  Serial.println();

  Serial.println("================================");

  Serial.println(" RAILWAY LINEN INVENTORY SYSTEM ");

  Serial.println("================================");

  Serial.println("Scan RFID Linen Tag...");
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  // ---------------------------------------------------
  // Check whether new RFID card is present
  // ---------------------------------------------------

  if (!rfid.PICC_IsNewCardPresent())
  {
    return;
  }


  // ---------------------------------------------------
  // Read RFID card
  // ---------------------------------------------------

  if (!rfid.PICC_ReadCardSerial())
  {
    return;
  }


  // ---------------------------------------------------
  // Create UID string
  // ---------------------------------------------------

  String uid = "";


  for (byte i = 0; i < rfid.uid.size; i++)
  {
    if (rfid.uid.uidByte[i] < 0x10)
    {
      uid += "0";
    }

    uid += String(
      rfid.uid.uidByte[i],
      HEX
    );
  }


  uid.toUpperCase();


  // ---------------------------------------------------
  // Display UID in Serial Monitor
  // ---------------------------------------------------

  Serial.println();

  Serial.println("=================================");

  Serial.print("RFID UID : ");

  Serial.println(uid);


  // ---------------------------------------------------
  // Search RFID in database
  // ---------------------------------------------------

  bool found = false;


  for (int i = 0; i < totalLinens; i++)
  {
    if (uid == linens[i].uid)
    {
      found = true;


      // ------------------------------------------------
      // Display linen details
      // ------------------------------------------------

      Serial.print("Linen ID : ");

      Serial.println(linens[i].linenID);


      Serial.print("Type     : ");

      Serial.println(linens[i].type);


      Serial.print("Location : ");

      Serial.println(linens[i].location);


      // ------------------------------------------------
      // Check current status
      // ------------------------------------------------

      String status;


      if (linens[i].checkedIn == false)
      {
        // ==============================================
        // ITEM IS AVAILABLE
        // ==============================================

        linens[i].checkedIn = true;

        status = "AVAILABLE";


        Serial.println("STATUS   : AVAILABLE");


        // Green LED

        greenLED();


        // LCD

        showAvailable(
          linens[i].linenID
        );
      }

      else
      {
        // ==============================================
        // ITEM IS MISSING / CHECKED OUT
        // ==============================================

        linens[i].checkedIn = false;

        status = "MISSING";


        Serial.println("STATUS   : MISSING");


        // Red LED

        redLED();


        // LCD

        showMissing(
          linens[i].linenID
        );
      }


      // ------------------------------------------------
      // Send information to Google Sheets
      // ------------------------------------------------

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


  // ===================================================
  // UNKNOWN RFID
  // ===================================================

  if (!found)
  {
    Serial.println("STATUS   : UNKNOWN RFID TAG");


    // Yellow LED

    yellowLED();


    // LCD

    showUnknown();
  }


  Serial.println("=================================");


  // ---------------------------------------------------
  // Stop RFID communication
  // ---------------------------------------------------

  rfid.PICC_HaltA();

  rfid.PCD_StopCrypto1();


  // ---------------------------------------------------
  // Wait
  // ---------------------------------------------------

  delay(3000);


  // ---------------------------------------------------
  // Return to ready screen
  // ---------------------------------------------------

  allLEDOff();

  showReadyScreen();
}