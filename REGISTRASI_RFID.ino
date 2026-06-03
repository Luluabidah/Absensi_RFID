#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 27

MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "";
const char* password = "";

String nim = "";
String nama = "";
String kelas = "";

String scriptURL =
"https://script.google.com/macros/s/AKfycbzVPL9faDCdGnMEDcOFebd0o4H9wJ8GtcVBgTBNCzug5C_AEJ2fKlbiTraUN8l7H5ymZQ/exec";

void setup() {

  Serial.begin(115200);

  SPI.begin();
  rfid.PCD_Init();

  Serial.println("Menghubungkan WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Terhubung");

  Serial.println();
  Serial.println("===== PENDAFTARAN KARTU =====");

  Serial.println("Masukkan NIM:");
}

void loop() {

  if (nim == "") {

    if (Serial.available()) {

      nim = Serial.readStringUntil('\n');
      nim.trim();

      Serial.println("Masukkan Nama:");
    }

    return;
  }

  if (nama == "") {

    if (Serial.available()) {

      nama = Serial.readStringUntil('\n');
      nama.trim();

      Serial.println("Masukkan Kelas:");
    }

    return;
  }

  if (kelas == "") {

    if (Serial.available()) {

      kelas = Serial.readStringUntil('\n');
      kelas.trim();

      Serial.println("Tempel Kartu RFID...");
    }

    return;
  }

  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10)
      uid += "0";

    uid += String(rfid.uid.uidByte[i], HEX);
  }

  uid.toUpperCase();

  Serial.println();
  Serial.println("Kartu Terbaca");
  Serial.println("UID : " + uid);

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url = scriptURL +
                 "?uid=" + uid +
                 "&nim=" + nim +
                 "&nama=" + nama +
                 "&kelas=" + kelas;

    url.replace(" ", "%20");

    Serial.println("Mengirim ke Spreadsheet...");

    http.begin(url);

    int httpCode = http.GET();

    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    String response = http.getString();

    Serial.println(response);

    http.end();
  }

  Serial.println();
  Serial.println("DATA BERHASIL DIKIRIM");
  Serial.println("--------------------");

  nim = "";
  nama = "";
  kelas = "";

  Serial.println("Masukkan NIM:");

  delay(3000);
}
