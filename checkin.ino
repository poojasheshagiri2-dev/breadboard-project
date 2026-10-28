#include <SPI.h>
#include <MFRC522.h>

//---------------- SPI Pins ----------------//
#define SS_PIN   10
#define RST_PIN  9

#define SPI_SCK  12
#define SPI_MISO 13
#define SPI_MOSI 11

MFRC522 rfid(SS_PIN, RST_PIN);


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


//------------------------------------------
// RFID Tags assigned to Linen
//------------------------------------------

Linen linens[] =
{
  {"F1EA9F53", "LINEN001", "Bed Sheet", "Coach A1", false},
  {"01939354", "LINEN002", "Blanket",   "Coach A1", false}
};

const int totalLinens = sizeof(linens) / sizeof(linens[0]);


//------------------------------------------
// SETUP
//------------------------------------------

void setup()
{
  Serial.begin(115200);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SS_PIN);

  rfid.PCD_Init();

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
  // Check for RFID card
  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;


  //------------------------------------------
  // Read UID
  //------------------------------------------

  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++)
  {
    if (rfid.uid.uidByte[i] < 0x10)
      uid += "0";

    uid += String(rfid.uid.uidByte[i], HEX);
  }

  uid.toUpperCase();


  //------------------------------------------
  // Display UID
  //------------------------------------------

  Serial.println();
  Serial.println("=================================");

  Serial.print("RFID UID : ");
  Serial.println(uid);


  //------------------------------------------
  // Search Linen Database
  //------------------------------------------

  bool found = false;

  for (int i = 0; i < totalLinens; i++)
  {
    if (uid == linens[i].uid)
    {
      found = true;


      //------------------------------------------
      // Display Linen Information
      //------------------------------------------

      Serial.print("Linen ID : ");
      Serial.println(linens[i].linenID);

      Serial.print("Type     : ");
      Serial.println(linens[i].type);

      Serial.print("Location : ");
      Serial.println(linens[i].location);


      //------------------------------------------
      // Check IN / Check OUT
      //------------------------------------------

      if (linens[i].checkedIn == false)
      {
        linens[i].checkedIn = true;

        Serial.println("STATUS   : CHECKED IN");
      }
      else
      {
        linens[i].checkedIn = false;

        Serial.println("STATUS   : CHECKED OUT");
      }

      break;
    }
  }


  //------------------------------------------
  // Unknown RFID
  //------------------------------------------

  if (!found)
  {
    Serial.println("STATUS   : UNKNOWN RFID TAG");
  }


  Serial.println("=================================");


  //------------------------------------------
  // Stop RFID communication
  //------------------------------------------

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1500);
}