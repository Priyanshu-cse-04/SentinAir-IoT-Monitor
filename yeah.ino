#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include "ThingSpeak.h" // <--- NEW LIBRARY

// ================= WIFI CONFIGURATION =================
const char* ssid = "Beastie";      // <--- CHANGE THIS
const char* password = "priyanshu@2004"; // <--- CHANGE THIS

// ================= THINGSPEAK CONFIGURATION =================
unsigned long myChannelNumber = 3239197;    // <--- CHANGE THIS (Your Channel ID)
const char * myWriteAPIKey = "ZVN2NO7ZBNH86UCO";   // <--- CHANGE THIS (Your Write API Key)

// ================= HARDWARE CONFIG =================
#define PMS_RX D5
#define PMS_TX D6
#define DHTPIN D7       
#define DHTTYPE DHT11
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ================= OBJECTS =================
SoftwareSerial pmsSerial(PMS_RX, PMS_TX);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WiFiClient  client; // WiFi Object

// ================= GLOBAL VARIABLES =================
float lastTemp = 0.0;
float lastHum = 0.0;
bool firstReadingDone = false; 

int rawPM25 = 0;
int rawPM10 = 0;
int actualAQI = 0;
int correctedAQI = 0;

struct pms5003data {
  uint16_t pm25_env;
  uint16_t pm100_env;
};
struct pms5003data data;

// Background Stars
const int numStars = 10; 
int starX[numStars];
int starY[numStars];
int starSpeed[numStars];

// Timers
unsigned long lastFrameTime = 0;
unsigned long lastDHTTime = 0;
unsigned long lastSwitchTime = 0;
unsigned long lastWiFiTime = 0; // Timer for Cloud Upload
bool showScreen1 = true;

// Eye Animation State
unsigned long lastBlinkTime = 0;
int blinkState = 0; 
long nextBlinkInterval = 3000; 

// --- EMOJI BITMAPS ---
const unsigned char maskFace [] PROGMEM = {
  0x07, 0xE0, 0x18, 0x18, 0x20, 0x04, 0x40, 0x02, 0x42, 0x42, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
  0x80, 0x01, 0xFF, 0xFF, 0x80, 0x01, 0x8A, 0x51, 0x40, 0x02, 0x20, 0x04, 0x18, 0x18, 0x07, 0xE0
};
const unsigned char happyFace [] PROGMEM = {
  0x07, 0xE0, 0x18, 0x18, 0x20, 0x04, 0x46, 0x62, 0x46, 0x62, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
  0x84, 0x21, 0x82, 0x41, 0x41, 0x82, 0x40, 0x02, 0x20, 0x04, 0x18, 0x18, 0x07, 0xE0, 0x00, 0x00
};
const unsigned char hotFace [] PROGMEM = {
  0x07, 0xE0, 0x18, 0x38, 0x20, 0x44, 0x42, 0x42, 0x42, 0x42, 0x80, 0x01, 0x80, 0x01, 0x80, 0x21,
  0x83, 0xC1, 0x84, 0x21, 0x48, 0x12, 0x48, 0x12, 0x20, 0x04, 0x18, 0x18, 0x07, 0xE0, 0x00, 0x00
};
const unsigned char coldFace [] PROGMEM = {
  0x07, 0xE0, 0x18, 0x18, 0x24, 0x24, 0x42, 0x42, 0x42, 0x42, 0x80, 0x01, 0x80, 0x01, 0x8A, 0xA1,
  0x85, 0x51, 0x8A, 0xA1, 0x40, 0x02, 0x40, 0x02, 0x20, 0x04, 0x18, 0x18, 0x07, 0xE0, 0x00, 0x00
};
const unsigned char* emojiArray[4] = {maskFace, happyFace, hotFace, coldFace};

// ================= HELPER FUNCTIONS =================
int calculateIndianAQI(int pm25) {
  if (pm25 < 0) pm25 = 0; 
  if (pm25 > 999) pm25 = 999; 
  if (pm25 <= 30)  return map(pm25, 0, 30, 0, 50);
  if (pm25 <= 60)  return map(pm25, 30, 60, 51, 100);
  if (pm25 <= 90)  return map(pm25, 60, 90, 101, 200);
  if (pm25 <= 120) return map(pm25, 90, 120, 201, 300);
  if (pm25 <= 250) return map(pm25, 120, 250, 301, 400);
  int aqi = map(pm25, 250, 380, 401, 500);
  if (aqi > 500) return 500;
  return aqi;
}

boolean readPMSdata(Stream *s) {
  if (!s->available()) return false;
  while (s->available() && s->peek() != 0x42) { s->read(); }
  if (s->available() < 32) return false;
  uint8_t buf[32];
  s->readBytes(buf, 32);
  if (buf[1] != 0x4D) return false;
  uint16_t calcChecksum = 0;
  for (uint8_t i = 0; i < 30; i++) calcChecksum += buf[i];
  uint16_t sentChecksum = makeWord(buf[30], buf[31]);
  if (calcChecksum != sentChecksum) return false; 
  data.pm25_env = makeWord(buf[12], buf[13]);
  data.pm100_env = makeWord(buf[14], buf[15]);
  return true;
}

// ================= GRAPHICS =================
void drawBaseEyes() {
  display.fillRoundRect(12, 24, 14, 20, 5, WHITE);
  display.fillRoundRect(34, 24, 14, 20, 5, WHITE);
}
void drawHappyEyes() {
  drawBaseEyes();
  display.fillCircle(21, 34, 3, BLACK);
  display.fillCircle(43, 34, 3, BLACK);
}
void drawBlinkEyes() {
  display.fillRect(12, 32, 14, 4, WHITE);
  display.fillRect(34, 32, 14, 4, WHITE);
}
void drawAngryEyes() {
  drawBaseEyes();
  display.fillCircle(19, 30, 3, BLACK);
  display.fillCircle(41, 30, 3, BLACK);
  display.fillTriangle(10, 20, 28, 20, 10, 32, BLACK);
  display.fillTriangle(50, 20, 32, 20, 50, 32, BLACK);
}

void animateEyes(bool isBadAir) {
  unsigned long currentMillis = millis();
  if (blinkState == 0) { 
      if (currentMillis - lastBlinkTime >= nextBlinkInterval) {
        blinkState = 1;
        lastBlinkTime = currentMillis;
      }
  } else { 
      if (currentMillis - lastBlinkTime >= 200) {
        blinkState = 0;
        lastBlinkTime = currentMillis;
        nextBlinkInterval = random(2000, 6000); 
      }
  }
  if (blinkState == 1) drawBlinkEyes(); 
  else {
      if (isBadAir) drawAngryEyes();
      else drawHappyEyes();
  }
}

void animateBackground() {
  for(int i=0; i<numStars; i++) {
    display.drawPixel(starX[i], starY[i], WHITE);
    starX[i] -= starSpeed[i];
    if(starX[i] < 0) {
      starX[i] = SCREEN_WIDTH;
      starY[i] = random(0, SCREEN_HEIGHT);
    }
  }
}

void drawCountdown() {
  unsigned long passed = millis() - lastSwitchTime;
  int secondsLeft = 5 - (passed / 1000);
  if(secondsLeft < 1) secondsLeft = 1; 
  if(secondsLeft > 5) secondsLeft = 5;
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 56); 
  display.print(secondsLeft);
  display.drawCircle(3, 59, 6, WHITE);
}

// ================= SETUP =================
void setup() {
  Serial.begin(9600);
  pmsSerial.begin(9600);
  dht.begin();
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) while(1);

  // --- CONNECT TO WIFI ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("Connecting to:");
  display.setCursor(0, 35);
  display.print(ssid);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wait for connection (max 10 seconds, don't block forever)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    display.print(".");
    display.display();
    attempts++;
  }

  ThingSpeak.begin(client);  // Initialize ThingSpeak
  
  // Init Stars
  randomSeed(analogRead(0));
  for(int i=0; i<numStars; i++) {
    starX[i] = random(0, SCREEN_WIDTH);
    starY[i] = random(0, SCREEN_HEIGHT);
    starSpeed[i] = random(1, 2); 
  }
  
  lastSwitchTime = millis(); 
  lastDHTTime = millis();
}

// ================= LOOP =================
void loop() {
  unsigned long now = millis();

  // --- 1. PMS SENSOR ---
  if (readPMSdata(&pmsSerial)) {
    rawPM25 = data.pm25_env;
    rawPM10 = data.pm100_env;
    if(rawPM25 > 999) rawPM25 = 999; 
    
    actualAQI = calculateIndianAQI(rawPM25);
    int humEffect = 0;
    if (firstReadingDone && lastHum > 30.0) {
       humEffect = (int)((lastHum - 30.0) * 0.33); 
    }
    int tempAdj = 0;
    if (firstReadingDone) {
      tempAdj = (int)((lastTemp - 25.0) * 0.5);
    }
    correctedAQI = actualAQI - humEffect + tempAdj;
    if (correctedAQI < 0) correctedAQI = 0; 
    if (correctedAQI > 500) correctedAQI = 500;
    if (correctedAQI == actualAQI && actualAQI > 10) correctedAQI = actualAQI - 2;
  }

  // --- 2. DHT SENSOR ---
  if (now - lastDHTTime >= 2500) {
    lastDHTTime = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && h > 0 && h <= 100) {
      lastHum = h;
      firstReadingDone = true;
    }
    if (!isnan(t) && t > -50 && t < 100) {
      lastTemp = t;
      firstReadingDone = true;
    }
  }

  // --- 3. UPLOAD TO CLOUD (Every 20 Seconds) ---
  if (now - lastWiFiTime >= 20000) { // 20000ms = 20s
    if (WiFi.status() == WL_CONNECTED) {
      // Set the 4 fields
      ThingSpeak.setField(1, rawPM25);      // Field 1: PM2.5
      ThingSpeak.setField(2, correctedAQI); // Field 2: AQI
      ThingSpeak.setField(3, lastTemp);     // Field 3: Temp
      ThingSpeak.setField(4, lastHum);      // Field 4: Humidity
      
      // Send data
      int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      
      // Update Timer
      lastWiFiTime = now;
    }
  }

  // --- 4. SCREEN TOGGLE ---
  if (now - lastSwitchTime >= 5000) { 
    lastSwitchTime = now;
    showScreen1 = !showScreen1;
  }

  // --- 5. DRAW FRAME (30FPS) ---
  if (now - lastFrameTime >= 33) {
    lastFrameTime = now;
    display.clearDisplay();
    animateBackground(); 
    
    // Tiny WiFi Icon in corner
    if (WiFi.status() == WL_CONNECTED) {
       display.fillCircle(124, 4, 2, WHITE); // Dot shows connected
    }

    if (showScreen1) {
      bool isBadAir = (rawPM25 > 60 || lastHum > 85);
      animateEyes(isBadAir);
      display.fillRect(62, 0, 66, 64, BLACK); 
      display.drawLine(62, 0, 62, 64, WHITE); 
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(66, 0); display.print("T:"); 
      if(firstReadingDone) display.print((int)lastTemp); else display.print("--");
      display.print("C");
      display.setCursor(66, 12); display.print("H:"); 
      if(firstReadingDone) display.print((int)lastHum); else display.print("--");
      display.print("%");
      display.drawLine(62, 24, 128, 24, WHITE); 
      display.setCursor(66, 30); display.print("PM2.5");
      display.setTextSize(2); 
      display.setCursor(66, 40); display.print(rawPM25);
      display.setTextSize(1);
      display.setCursor(66, 56); display.print("PM10: "); display.print(rawPM10);
    } 
    else {
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(0,0); display.print("0");
      display.setCursor(110,0); display.print("500");
      display.drawRect(0, 10, 128, 8, WHITE); 
      int barW = map(correctedAQI, 0, 500, 0, 126);
      if(barW > 126) barW = 126;
      display.fillRect(2, 12, barW, 4, WHITE);
      display.setCursor(0, 25); display.print("ACTUAL RAW:"); display.print(actualAQI);
      display.setCursor(0, 42); display.print("REAL AQI:");
      display.fillRect(55, 36, 73, 24, WHITE); 
      display.setTextColor(BLACK);
      display.setTextSize(2); 
      if(correctedAQI < 10) display.setCursor(80, 40);
      else if(correctedAQI < 100) display.setCursor(75, 40);
      else display.setCursor(65, 40);
      display.print(correctedAQI);
    }
    
    display.display();
  }
}