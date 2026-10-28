function doGet(e) {

  // PUT YOUR GOOGLE SHEET ID HERE
  var sheet = SpreadsheetApp
    .openById("19MVV5Yba7dNR4FwnMbKPOs6WZhJpd_C4_ZiaAPumqic")
    .getSheetByName("Sheet1");

  // Data received from ESP32
  var uid = e.parameter.uid || "";
  var linenID = e.parameter.linenID || "";
  var type = e.parameter.type || "";
  var location = e.parameter.location || "";
  var status = e.parameter.status || "";

  // Add data to Google Sheet
  sheet.appendRow([
    new Date(),     // Date & Time
    uid,            // RFID UID
    linenID,        // Linen ID
    type,           // Item
    location,       // Coach/Location
    status          // Status
  ]);

  return ContentService
    .createTextOutput("Data received successfully");
}
