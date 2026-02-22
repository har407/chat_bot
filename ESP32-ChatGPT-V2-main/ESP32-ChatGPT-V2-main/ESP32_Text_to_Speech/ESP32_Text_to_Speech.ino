/*
   ESP32 Text-to-Speech from UART (ESP32 ↔ ESP32)
   Uses Google TTS via ESP32-audioI2S

   Requirements:
   - ESP32 Board Package v1.0.6
   - ESP32-audioI2S by schreibfaul1
   - Arduino IDE 1.8.19
*/

#include <Arduino.h>
#include <WiFi.h>
#include "Audio.h"   // https://github.com/schreibfaul1/ESP32-audioI2S

// ===== UART =====
#define RXp2 16
#define TXp2 17

// ===== I2S PINS =====
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC  26

// ===== WiFi =====
const char *ssid     = "SSID";
const char *password = "PASS";

// ===== Audio =====
Audio audio;

// ===== Buffers =====
String uartBuffer = "";
bool isSpeaking = false;

// ===== SETTINGS =====
const size_t CHUNK_SIZE = 80;   // Safe size for Google TTS

// ----------------------------------------------------

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXp2, TXp2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(100);

  audio.connecttospeech("System started", "en");
}

// ----------------------------------------------------

void loop() {
  audio.loop();   // MUST be called repeatedly

  // ===== Read UART safely =====
  while (Serial2.available()) {
    char c = Serial2.read();

    if (c == '\n') {          // End of message
      speakText(uartBuffer);
      uartBuffer = "";
    } else {
      uartBuffer += c;
    }
  }
}

// ----------------------------------------------------

void speakText(const String &text) {
  if (text.length() == 0) return;

  Serial.println("Received Text:");
  Serial.println(text);

  for (size_t i = 0; i < text.length(); i += CHUNK_SIZE) {
    String chunk = text.substring(i, i + CHUNK_SIZE);
    Serial.println("Speaking chunk:");
    Serial.println(chunk);

    audio.connecttospeech(chunk.c_str(), "en");

    while (audio.isRunning()) {
      audio.loop();   // keep audio alive
      delay(1);       // yield to system
    }
  }
}

// ----------------------------------------------------

void audio_info(const char *info) {
  // Optional debug
  // Serial.println(info);
}
