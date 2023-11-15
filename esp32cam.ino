#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Ganti dengan SSID dan password Wi-Fi Anda
const char* ssid = "nop";
const char* password = "hehehehe";

// Alamat server yang akan menerima gambar
const char* serverName = "http://azure/upload";

// Pin konfigurasi untuk ESP32-CAM (AI Thinker model)
#define CAMERA_MODEL_AI_THINKER

void setup() {
  Serial.begin(115200);

 // Konfigurasi kamera untuk AI-Thinker ESP32-CAM
    const int pinD0 = 5;
    const int pinD1 = 18;
    const int pinD2 = 19;
    const int pinD3 = 21;
    const int pinD4 = 36;
    const int pinD5 = 39;
    const int pinD6 = 34;
    const int pinD7 = 35;
    const int pinXCLK = 0;
    const int pinPCLK = 22;
    const int pinVSYNC = 25;
    const int pinHREF = 23;
    const int pinSDA = 26;
    const int pinSCL = 27;
    const int pinPWDN = 32;
    const int pinRESET = -1;

    // Inisialisasi kamera dengan konfigurasi yang sesuai
camera_config_t config = {
  .pin_pwdn = pinPWDN,
  .pin_reset = pinRESET,
  .pin_xclk = pinXCLK,
  .pin_sscb_sda = pinSDA,
  .pin_sscb_scl = pinSCL,
  .pin_d7 = pinD7,
  .pin_d6 = pinD6,
  .pin_d5 = pinD5,
  .pin_d4 = pinD4,
  .pin_d3 = pinD3,
  .pin_d2 = pinD2,
  .pin_d1 = pinD1,
  .pin_d0 = pinD0,
  .pin_vsync = pinVSYNC,
  .pin_href = pinHREF,
  .pin_pclk = pinPCLK,
  .xclk_freq_hz = 20000000,
  .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,
  .pixel_format = PIXFORMAT_JPEG,
  // pengaturan resolusi gambar tergantung pada PSRAM
  .frame_size = FRAMESIZE_UXGA,
  .jpeg_quality = 10,
  .fb_count = 2
  };

  // Kamera inisialisasi
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init gagal dengan kesalahan 0x%x", err);
    return;
  }

  // Sambungkan ke Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung");
}

void loop() {
  // Mengambil foto
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Pengambilan gambar gagal");
    return;
  }

  // Mengirim foto ke server
  Serial.println("Menghubungkan ke server...");
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "image/jpeg");
  
  Serial.println("Mengirim gambar...");
  int httpResponseCode = http.POST(fb->buf, fb->len);

  if (httpResponseCode == 200) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  esp_camera_fb_return(fb);

  
  delay(5000); 
}
