//
// mimoLive Tally Client
// *********************
// https://github.com/RedDevTech
//
// CPU:     Adafruit 3213 https://www.adafruit.com/product/3213 (Feather HUZZAH ESP8266 WiFi)
//          
// Board:   Use internal LED only (lights up if mimolive_url_video goes live, no support for mimolive_url_audio)
//          or Adafruit 2945 https://www.adafruit.com/product/2945 (NeoPixel FeatherWing Add-on, Note: active pin has to be changed to 15)
//          or Adafruit 3163 https://www.adafruit.com/product/3163 (Adafruit 15x7 CharliePlex LED Matrix FeatherWing)
//
// Code:    1) Set hardware settings:
//             for internal LED only: set boardtype to 0
//             for Adafruit 2945: set boardtype to 2945, neopixels_pin, neopixels_brightness
//             for Adafruit 3163: set boardtype to 3163, charlieplex_brightness_high, charlieplex_brightness_low
//          2) Set wifi credentials: wifi_ssid, wifi_pass
//          3) Set mimoLive api endpoints: mimolive_url_audio, mimolive_url_video and mimolive_timeout
//
// Version: 1.0 (20200519) First version with hardcoded mimoLive API endpoints for audio and video layer url
//
//

// user settings
#define wifi_ssid "wifissid"
#define wifi_pass "wifipass"
#define mimolive_url_audio "http://127.0.0.1:8989/api/v1/documents/1234567890/layers/ABCDEFGH-1234-5678-9012-ABCDEFGHIJKL"
#define mimolive_url_video "http://127.0.0.1:8989/api/v1/documents/1234567890/layers/ABCDEFGH-1234-5678-9012-ABCDEFGHIJKL"
#define mimolive_timeout 1000

// board settings
#define boardtype 3163
#define charlieplex_brightness_high 128
#define charlieplex_brightness_low 64
#define neopixels_pin 15
#define neopixels_brightness 64

// debug settings
#define debug 0     // 0: no serial debug, 1: status messages on serial, 2: all messages on serial

// includes
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// init boards
#define USE_SERIAL Serial
#define neopixels_pixels 32
Adafruit_NeoPixel pixels(neopixels_pixels, neopixels_pin, NEO_GRB + NEO_KHZ800);
Adafruit_IS31FL3731_Wing matrix = Adafruit_IS31FL3731_Wing();
ESP8266WiFiMulti WiFiMulti;

// symbols
static const uint8_t PROGMEM
  audio_off_bmp[] =
  { B00000000,
    B01111100,
    B01000100,
    B01111100,
    B01000100,
    B01000100,
    B00000000 },
  audio_live_bmp[] =
  { B11111110,
    B10000010,
    B10111010,
    B10000010,
    B10111010,
    B10111010,
    B11111110 },
  video_off_bmp[] =
  { B00000000,
    B01000100,
    B01000100,
    B01000100,
    B00101000,
    B00010000,
    B00000000 },
  video_live_bmp[] =
  { B11111110,
    B10111010,
    B10111010,
    B10111010,
    B11010110,
    B11101110,
    B11111110 };

// functions
void showStart() {
  if (boardtype == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
  }
  if (boardtype == 2945) {
    pixels.clear();
    pixels.setPixelColor(0, 255, 0, 0);
    pixels.setPixelColor(31, 0, 255, 0);
    pixels.show();
  }
  if (boardtype == 3163) {
    matrix.clear();
    matrix.drawPixel(0, 0, charlieplex_brightness_high);
    matrix.drawPixel(14, 0, charlieplex_brightness_low);
    matrix.drawPixel(0, 6, charlieplex_brightness_high);
    matrix.drawPixel(14, 6, charlieplex_brightness_low);
  }
}
void showAudio(bool live) {
  if (live) {
    if (debug) { USE_SERIAL.print("[BOARD] SHOW AUDIO: LIVE\n"); }
    if (boardtype == 2945) {
      pixels.setPixelColor(0, 255, 0, 0);
      pixels.setPixelColor(1, 255, 0, 0);
      pixels.setPixelColor(2, 255, 0, 0);
      pixels.setPixelColor(3, 0, 0, 0);
      pixels.setPixelColor(8, 255, 0, 0);
      pixels.setPixelColor(9, 0, 0, 0);
      pixels.setPixelColor(10, 255, 0, 0);
      pixels.setPixelColor(11, 0, 0, 0);
      pixels.setPixelColor(16, 255, 0, 0);
      pixels.setPixelColor(17, 255, 0, 0);
      pixels.setPixelColor(18, 255, 0, 0);
      pixels.setPixelColor(19, 0, 0, 0);
      pixels.setPixelColor(24, 255, 0, 0);
      pixels.setPixelColor(25, 0, 0, 0);
      pixels.setPixelColor(26, 255, 0, 0);
      pixels.setPixelColor(27, 0, 0, 0);
      pixels.show();
    }
    if (boardtype == 3163) {
      matrix.drawBitmap(0, 0, audio_off_bmp, 8, 7, 0);
      matrix.drawBitmap(0, 0, audio_live_bmp, 8, 7, charlieplex_brightness_high);
    }
  } else {
    if (debug) { USE_SERIAL.print("[BOARD] SHOW AUDIO: N/A\n"); }
    if (boardtype == 2945) {
      pixels.setPixelColor(0, 0, 255, 0);
      pixels.setPixelColor(1, 0, 255, 0);
      pixels.setPixelColor(2, 0, 255, 0);
      pixels.setPixelColor(3, 0, 0, 0);
      pixels.setPixelColor(8, 0, 255, 0);
      pixels.setPixelColor(9, 0, 0, 0);
      pixels.setPixelColor(10, 0, 255, 0);
      pixels.setPixelColor(11, 0, 0, 0);
      pixels.setPixelColor(16, 0, 255, 0);
      pixels.setPixelColor(17, 0, 255, 0);
      pixels.setPixelColor(18, 0, 255, 0);
      pixels.setPixelColor(19, 0, 0, 0);
      pixels.setPixelColor(24, 0, 255, 0);
      pixels.setPixelColor(25, 0, 0, 0);
      pixels.setPixelColor(26, 0, 255, 0);
      pixels.setPixelColor(27, 0, 0, 0);
      pixels.show();
    }
    if (boardtype == 3163) {
      matrix.drawBitmap(0, 0, audio_live_bmp, 8, 7, 0);
      matrix.drawBitmap(0, 0, audio_off_bmp, 8, 7, charlieplex_brightness_low);
    }
  }
}
void showVideo(bool live) {
  if (live) {
    if (debug) { USE_SERIAL.print("[BOARD] SHOW VIDEO: LIVE\n"); }
    if (boardtype  == 0) {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    if (boardtype == 2945) {
      pixels.setPixelColor(4, 0, 0, 0);
      pixels.setPixelColor(5, 255, 0, 0);
      pixels.setPixelColor(6, 0, 0, 0);
      pixels.setPixelColor(7, 255, 0, 0);
      pixels.setPixelColor(12, 0, 0, 0);
      pixels.setPixelColor(13, 255, 0, 0);
      pixels.setPixelColor(14, 0, 0, 0);
      pixels.setPixelColor(15, 255, 0, 0);
      pixels.setPixelColor(20, 0, 0, 0);
      pixels.setPixelColor(21, 255, 0, 0);
      pixels.setPixelColor(22, 0, 0, 0);
      pixels.setPixelColor(23, 255, 0, 0);
      pixels.setPixelColor(28, 0, 0, 0);
      pixels.setPixelColor(29, 0, 0, 0);
      pixels.setPixelColor(30, 255, 0, 0);
      pixels.setPixelColor(31, 0, 0, 0);
      pixels.show();
    }
    if (boardtype == 3163) {
      matrix.drawBitmap(8, 0, video_off_bmp, 8, 7, 0);
      matrix.drawBitmap(8, 0, video_live_bmp, 8, 7, charlieplex_brightness_high);
    }   
  } else {
    if (debug) { USE_SERIAL.print("[BOARD] SHOW VIDEO: N/A\n"); }
    if (boardtype  == 0) {
      digitalWrite(LED_BUILTIN, LOW);
    }
    if (boardtype == 2945) {
      pixels.setPixelColor(4, 0, 0, 0);
      pixels.setPixelColor(5, 0, 255, 0);
      pixels.setPixelColor(6, 0, 0, 0);
      pixels.setPixelColor(7, 0, 255, 0);
      pixels.setPixelColor(12, 0, 0, 0);
      pixels.setPixelColor(13, 0, 255, 0);
      pixels.setPixelColor(14, 0, 0, 0);
      pixels.setPixelColor(15, 0, 255, 0);
      pixels.setPixelColor(20, 0, 0, 0);
      pixels.setPixelColor(21, 0, 255, 0);
      pixels.setPixelColor(22, 0, 0, 0);
      pixels.setPixelColor(23, 0, 255, 0);
      pixels.setPixelColor(28, 0, 0, 0);
      pixels.setPixelColor(29, 0, 0, 0);
      pixels.setPixelColor(30, 0, 255, 0);
      pixels.setPixelColor(31, 0, 0, 0);
      pixels.show();
    }
    if (boardtype == 3163) {
      matrix.drawBitmap(8, 0, video_live_bmp, 8, 7, 0);
      matrix.drawBitmap(8, 0, video_off_bmp, 8, 7, charlieplex_brightness_low);
    }   
  }
}

// setup
void setup() {

    // serial
    if (debug) {
      USE_SERIAL.begin(115200);
      if (debug >= 2) { USE_SERIAL.setDebugOutput(true); }
      USE_SERIAL.println();
      USE_SERIAL.println();
      USE_SERIAL.println();
      for (uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
      }
    }

    // board
    if (boardtype == 0) {
      pinMode(LED_BUILTIN, OUTPUT);
    }
    if (boardtype == 2945) {
      pixels.begin();
      pixels.setBrightness(neopixels_brightness);
    }
    if (boardtype == 3163) {
      if (matrix.begin()) {
        matrix.setRotation(0);
      } else {
        USE_SERIAL.println("IS31 not found");
      }
    }
    showStart();

    // wifi
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(wifi_ssid, wifi_pass);

    // reset board
    if (boardtype == 0) {
      digitalWrite(LED_BUILTIN, LOW);
    }
    if (boardtype == 2945) {
      pixels.clear();
      pixels.show();
    }
    if (boardtype == 3163) {
      matrix.clear();
    }

}

// loop
void loop() {
  
    // check wifi
    if ((WiFiMulti.run() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        if (debug) { USE_SERIAL.print("[HTTP] begin...\n"); }

        // audio
        if (mimolive_url_audio != "") {
          http.begin(client, mimolive_url_audio);
          if (debug) { USE_SERIAL.print("[HTTP] GET AUDIO...\n"); }
          int httpCode = http.GET();
          if (httpCode > 0) {
              if (debug) { USE_SERIAL.printf("[HTTP] GET AUDIO... code: %d\n", httpCode); }
              if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                String payload = http.getString();
                if (debug >= 2) { USE_SERIAL.println(payload); USE_SERIAL.println(payload.indexOf("\"live-state\":\"live\"")); }
                if (payload.indexOf("\"live-state\":\"live\"") >= 0) {
                  showAudio(true);
                } else {
                  showAudio(false);
                }
              }
          } else {
            if (debug) { USE_SERIAL.printf("[HTTP] GET AUDIO... failed, error: %s\n", http.errorToString(httpCode).c_str()); }
          }
          http.end();
        }

        // video
        if (mimolive_url_video != "") {
          http.begin(client, mimolive_url_video);
          if (debug) { USE_SERIAL.print("[HTTP] GET VIDEO...\n"); }
          int httpCode = http.GET();
          if (httpCode > 0) {
              if (debug) { USE_SERIAL.printf("[HTTP] GET VIDEO... code: %d\n", httpCode); }
              if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                int len = http.getSize();
                uint8_t buff[128] = { 0 };
                WiFiClient *stream = &client;
                bool live = false;
                String payload;
                while (http.connected() && (len > 0 || len == -1)) {
                  int c = stream->readBytes(buff, std::min((size_t)len, sizeof(buff)));
                  payload = (char*)buff;
                  if (debug >= 2) { USE_SERIAL.write(buff, c); USE_SERIAL.println(payload.indexOf("\"live-state\":\"live\"")); }
                  if (payload.indexOf("\"live-state\":\"live\"") >= 0) { live = true; }
                  if (len > 0) { len -= c; }
                }
                showVideo(live);
              }
          } else {
              if (debug) { USE_SERIAL.printf("[HTTP] GET VIDEO... failed, error: %s\n", http.errorToString(httpCode).c_str()); }
          }
          http.end();
        }
        
    }

    delay(mimolive_timeout);
}
