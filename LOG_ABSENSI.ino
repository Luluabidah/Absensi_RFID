#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 5
#define RST_PIN 27

#define LED_HIJAU 14
#define LED_MERAH 12
#define BUZZER 13

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "vivo Y27s";
const char* password = "fighting123";

String scriptURL =
"https://script.google.com/macros/s/AKfycbzVPL9faDCdGnMEDcOFebd0o4H9wJ8GtcVBgTBNCzug5C_AEJ2fKlbiTraUN8l7H5ymZQ/exec";

void bunyiBerhasil() {
  tone(BUZZER, 2000);
  delay(300);
  noTone(BUZZER);
}

void bunyiGagal() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER, 2000);
    delay(200);
    noTone(BUZZER);
    delay(200);
  }
}

void tampilStandby() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SISTEM ABSENSI");
  lcd.setCursor(0, 1);
  lcd.print("TEMPEL KARTU");
}

void setup() {

  Serial.begin(115200);

  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(LED_HIJAU, LOW);
  digitalWrite(LED_MERAH, LOW);

  // LCD
  Wire.begin(16, 17);
  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MENGHUBUNGKAN");
  lcd.setCursor(0, 1);
  lcd.print("WIFI...");

  // RFID
  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();

  byte versi = rfid.PCD_ReadRegister(MFRC522::VersionReg);

  Serial.print("Versi RC522: 0x");
  Serial.println(versi, HEX);

  Serial.println("Menghubungkan WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Terhubung");

  tampilStandby();

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
  Serial.println("Kartu Terbaca");
  Serial.println("UID : " + uid);

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url = scriptURL + "?uid=" + uid;

    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    http.begin(url);

unsigned long mulai = millis();

int httpCode = http.GET();

unsigned long selesai = millis();

Serial.print("Waktu HTTP : ");
Serial.print(selesai - mulai);
Serial.println(" ms");

Serial.print("HTTP Code : ");
Serial.println(httpCode);

String response = http.getString();
response.trim();

    Serial.print("Response : ");
    Serial.println(response);

    if (response.startsWith("ABSEN_BERHASIL")) {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ABSEN BERHASIL");

      String nama = "";

      int posisi = response.indexOf('|');

      if (posisi > 0) {
        nama = response.substring(posisi + 1);
      }

      lcd.setCursor(0, 1);

      if (nama.length() > 16) {
        nama = nama.substring(0, 16);
      }

      lcd.print(nama);

      digitalWrite(LED_HIJAU, HIGH);

      bunyiBerhasil();

      delay(2000);

      digitalWrite(LED_HIJAU, LOW);

      Serial.println("ABSENSI BERHASIL");
    }

    else if (response == "TIDAK_TERDAFTAR") {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("KARTU TIDAK");

      lcd.setCursor(0, 1);
      lcd.print("TERDAFTAR");

      digitalWrite(LED_MERAH, HIGH);

      bunyiGagal();

      delay(2000);

      digitalWrite(LED_MERAH, LOW);

      Serial.println("KARTU TIDAK TERDAFTAR");
    }

    else {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ERROR RESPON");

      lcd.setCursor(0, 1);
      lcd.print("CEK SERVER");

      Serial.println("RESPON TIDAK DIKENAL");
      Serial.println(response);
    }

    http.end();
  }

  rfid.PICC_HaltA();

  delay(1000);

  tampilStandby();

  Serial.println();
  Serial.println("Tempel Kartu...");
}
