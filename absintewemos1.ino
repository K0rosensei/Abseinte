#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// Informasi Wi-Fi
const char* ssid = "nop";     // nama SSID jaringan Wi-Fi
const char* password = "hehehehe"; // password Wi-Fi

// Konfigurasi RFID RC522
#define RST_PIN D3  // Pin RST dari RC522
#define SS_PIN  D4  // Pin SDA dari RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); // instansi MFRC522

// Konfigurasi server
const char* serverName = "http://192.168.1.4:100/absente_V.1.1.2/post-rfid-data"; // alamat server

// Inisialisasi LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  

WiFiClient client;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  lcd.init();                      
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Silahkan dekatkan");
  lcd.setCursor(0, 1);
  lcd.print("kartu");
}

void loop() {
  // Cari kartu baru
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50); // Jika tidak ada kartu, delay sebentar kemudian coba lagi
    return;
  }

  String rfidTag = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidTag += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
  }
  rfidTag.toUpperCase();

  Serial.print("RFID Tag: ");
  Serial.println(rfidTag);

  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = "rfid=" + rfidTag; 
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString(); 
      Serial.println(payload);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dekatkan wajah");
      delay(3000); // Tunggu 3 detik sebelum mengambil gambar
      
      // Fungsi mengirim perintah ke ESP32-CAM untuk mengambil gambar
      sendCaptureCommand();

      // Tampilkan pesan mohon tunggu di LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Please wait...");

      // Tunggu respon dari server untuk verifikasi
      httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        String serverResponse = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(serverResponse);

        lcd.clear();
        if (serverResponse.indexOf("Success") >= 0) {
          lcd.print("Absen Berhasil");
        } else {
          lcd.print("Absen Gagal");
        }
      } else {
        Serial.println("Error on HTTP request");
      }
      http.end(); // Tutup koneksi HTTP
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
  } else {
    Serial.println("WiFi Disconnected");
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(2000);
}

void sendCaptureCommand() {
  
  const char* esp32_cam_ip = "http://192.168.137.225/capture"; 
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, esp32_cam_ip);
    int httpCode = http.GET();
  
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  }
}
