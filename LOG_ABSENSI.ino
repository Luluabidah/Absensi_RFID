#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 27

#define LED_HIJAU 14
#define LED_MERAH 12
#define BUZZER 13

MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "";
const char* password = "";

String scriptURL =
"https://script.google.com/macros/s/AKfycbzVPL9faDCdGnMEDcOFebd0o4H9wJ8GtcVBgTBNCzug5C_AEJ2fKlbiTraUN8l7H5ymZQ/exec";

void setup() {

  Serial.begin(115200);

  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(LED_HIJAU, LOW);
  digitalWrite(LED_MERAH, LOW);

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
  Serial.println("Sistem Absensi Siap");
  Serial.println("Tempel Kartu...");
}

void loop() {

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
  Serial.println("UID : " + uid);

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url = scriptURL +
                 "?uid=" + uid;

    Serial.println("Mengirim...");
    Serial.println(url);

    http.begin(url);

    int httpCode = http.GET();

    Serial.print("HTTP Code : ");
    Serial.println(httpCode);

    String response = http.getString();

    Serial.println("Response:");
    Serial.println(response);

    // =========================
    // ABSEN BERHASIL
    // =========================

    if (response.indexOf("ABSEN_BERHASIL") >= 0) {

      digitalWrite(LED_HIJAU, HIGH);

      tone(BUZZER, 2000);
      delay(200);
      noTone(BUZZER);

      delay(1000);

      digitalWrite(LED_HIJAU, LOW);

      Serial.println("ABSEN BERHASIL");
    }

    // =========================
    // TIDAK TERDAFTAR
    // =========================

    else if (response.indexOf("TIDAK_TERDAFTAR") >= 0) {

      digitalWrite(LED_MERAH, HIGH);

      for (int i = 0; i < 3; i++) {

        tone(BUZZER, 2000);
        delay(200);
        noTone(BUZZER);
        delay(200);
      }

      delay(1000);

      digitalWrite(LED_MERAH, LOW);

      Serial.println("KARTU TIDAK TERDAFTAR");
    }

    http.end();
  }

  rfid.PICC_HaltA();

  delay(2000);
}
