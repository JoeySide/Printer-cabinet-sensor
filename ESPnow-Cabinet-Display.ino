#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library

#define TFT_DC 18
#define TFT_CS 16
#define TFT_RST 0
#define TFT_SCLK 35
#define TFT_MOSI 33

char MAC_DIS[18] = "C0:4E:30:4B:19:BC";
char MAC_FIL[18] = "C0:4E:30:4B:2F:8C";
char MAC_PR1[18] = "48:27:E2:46:C4:16";
char MAC_PR2[18] = "48:27:E2:47:35:5C";
char MAC_GAR[18] = "C0:4E:30:4B:19:E0";

char lastTempFIL[3]= "99";
char lastHumFIL[3]= "99";
char lastTempPR1[3]= "99";
char lastHumPR1[3]= "99";
char lastTempPR2[3]= "99";
char lastHumPR2[3]= "99";
char lastTempGAR[3]= "99";
char lastHumGAR[3]= "99";

int updateTimer = 0;

int lastUpdateFIL = 0;
int lastUpdatePR1 = 0;
int lastUpdatePR2 = 0;
int lastUpdateGAR = 0;

bool lostFIL = false;
bool lostPR1 = false;
bool lostPR2 = false;
bool lostGAR = false;

int textStart = 36;
int textDataStart = 55;
int textUnitStart = 70;

typedef struct struct_message {
  char mac[18];
  int temperature;
  int humidity;
} struct_message;

struct_message myData;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void OnDataRecv(const esp_now_recv_info *recInfo, const uint8_t *incomingData, int len) {
  Serial.println("Receiving Data!");
  memcpy(&myData, incomingData, sizeof(myData));
  char recMac[18];
  char temp[3];
  char hum[3];

  strcpy(recMac, myData.mac);
  itoa(myData.temperature, temp, 10);
  itoa(myData.humidity, hum, 10);
  
  Serial.print("Data received from: ");
  Serial.print(recMac);
  Serial.print("   Data -> Temp: ");
  Serial.print(temp);
  Serial.print(" - Hum: ");
  Serial.println(hum);

  if(strcmp(recMac,MAC_FIL)==0){
    drawFilTemp(temp);
    drawFilHum(hum);
    lastUpdateFIL = 0;
  }
  else if(strcmp(recMac,MAC_PR1)==0){
    drawPr1Temp(temp);
    drawPr1Hum(hum);
    lastUpdatePR1 = 0;
  }
  else if(strcmp(recMac,MAC_PR2)==0){
    drawPr2Temp(temp);
    drawPr2Hum(hum);
    lastUpdatePR2 = 0;
  }
  else if(strcmp(recMac,MAC_GAR)==0){
    drawGarageTemp(temp);
    drawGarageHum(hum);
    lastUpdateGAR = 0;
  }
}


void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }
  WiFi.mode(WIFI_STA);

  if(esp_now_init() != ESP_OK){
    Serial.println("Error initializing ESP-NOW");
    return;
  } else { Serial.println("ESP-NOW initialized");}


  textDataStart = textStart + 38;
  textUnitStart = textStart + 52;

  tft.initR(INITR_GREENTAB);
  tft.fillScreen(ST7735_BLACK);
  drawTemplate();

  if(esp_now_register_recv_cb(OnDataRecv) != ESP_OK) {
    Serial.println("Failed to Register Receiver Callback");
  } else { Serial.println("Receiver Callback registered"); }

  Serial.println("Dispay started");
}

void loop() {
  if(updateTimer + 1000 < millis())
  {
    if(updateTimer > millis()){ updateTimer = 0; }
    updateTimer = millis();
    lastUpdateFIL++;
    lastUpdatePR1++;
    lastUpdatePR2++;
    lastUpdateGAR++;
    if(lastUpdateFIL > 15) { if(!lostFIL) { changeFilColor(ST7735_BLUE); lostFIL = true; } }
    else { if(lostFIL) { changeFilColor(ST7735_BLACK); lostFIL = false; } }

    if(lastUpdatePR1 > 15) { if(!lostPR1) { changePR1Color(ST7735_BLUE); lostPR1 = true; } }
    else { if(lostPR1) { changePR1Color(ST7735_BLACK); lostPR1 = false; } }

    if(lastUpdatePR2 > 15) { if(!lostPR2) { changePR2Color(ST7735_BLUE); lostPR2 = true; } }
    else { if(lostPR2) { changePR2Color(ST7735_BLACK); lostPR2 = false; } }

    if(lastUpdateGAR > 15) { if(!lostGAR) { changeGARColor(ST7735_BLUE); lostGAR = true; } }
    else { if(lostGAR) { changeGARColor(ST7735_BLACK); lostGAR = false; } }
  }
}

void drawTemplate() {
  tft.fillRect(-1, -1, tft.width(), 13, ST7735_WHITE);
  tft.setCursor(38, 3);
  tft.setTextColor(ST7735_BLACK);
  tft.print("FILAMENTS");
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(textStart, 16);
  tft.print("Temp: ");
  tft.setCursor(textStart, 26);
  tft.print("Hum: ");
  tft.setCursor(textUnitStart, 16);
  tft.print("C");
  tft.setCursor(textUnitStart, 26);
  tft.print("%");

  tft.fillRect(-1, 40, tft.width(), 12, ST7735_WHITE);
  tft.setCursor(38, 42);
  tft.setTextColor(ST7735_BLACK);
  tft.print("PRINTER 1");
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(textStart, 56);
  tft.print("Temp: ");
  tft.setCursor(textStart, 66);
  tft.print("Hum: ");
  tft.setCursor(textUnitStart, 56);
  tft.print("C");
  tft.setCursor(textUnitStart, 66);
  tft.print("%");

  tft.fillRect(-1, 79, tft.width(), 12, ST7735_WHITE);
  tft.setCursor(38, 81);
  tft.setTextColor(ST7735_BLACK);
  tft.print("PRINTER 2");
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(textStart, 96);
  tft.print("Temp: ");
  tft.setCursor(textStart, 106);
  tft.print("Hum: ");
  tft.setCursor(textUnitStart, 96);
  tft.print("C");
  tft.setCursor(textUnitStart, 106);
  tft.print("%");

  tft.fillRect(-1, 118, tft.width(), 12, ST7735_WHITE);
  tft.setCursor(47, 120);
  tft.setTextColor(ST7735_BLACK);
  tft.print("GARAGE");
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(textStart, 136);
  tft.print("Temp: ");
  tft.setCursor(textStart, 146);
  tft.print("Hum: ");
  tft.setCursor(textUnitStart, 136);
  tft.print("C");
  tft.setCursor(textUnitStart, 146);
  tft.print("%");
}

void changeFilColor(uint16_t color){
  tft.setCursor(38, 3);
  tft.setTextColor(color);
  tft.print("FILAMENTS");
  tft.setTextColor(ST7735_WHITE);
}

void changePR1Color(uint16_t color){
  tft.setCursor(38, 42);
  tft.setTextColor(color);
  tft.print("PRINTER 1");
  tft.setTextColor(ST7735_WHITE);
}

void changePR2Color(uint16_t color){
  tft.setCursor(38, 81);
  tft.setTextColor(color);
  tft.print("PRINTER 2");
  tft.setTextColor(ST7735_WHITE);
}

void changeGARColor(uint16_t color){
  tft.setCursor(47, 120);
  tft.setTextColor(color);
  tft.print("GARAGE");
  tft.setTextColor(ST7735_WHITE);
}

void drawFilTemp(char value[3]){
  if(strcmp(lastTempFIL, value)==0)
    return;
  strcpy(lastTempFIL,value);
  tft.fillRect(textDataStart, 16, 12, 10, ST7735_BLACK);
  tft.setCursor(textDataStart, 16);
  tft.print(value);
}
void drawFilHum(char value[3]){
  if(strcmp(lastHumFIL, value)==0)
    return;
  strcpy(lastHumFIL,value);
  tft.fillRect(textDataStart, 26, 12, 10, ST7735_BLACK);
  tft.setCursor(textDataStart, 26);
  tft.print(value);
}
void drawPr1Temp(char value[3]){
  if(strcmp(lastTempPR1, value)==0)
    return;
  strcpy(lastTempPR1,value);
  tft.fillRect(textDataStart, 56, 12, 10, ST7735_BLACK);
  tft.setCursor(textDataStart, 56);
  tft.print(value);
}
void drawPr1Hum(char value[3]){
  if(strcmp(lastHumPR1, value)==0)
    return;
  strcpy(lastHumPR1,value);
  tft.fillRect(textDataStart, 66, 12, 10, ST7735_BLACK);
  tft.setCursor(textDataStart, 66);
  tft.print(value);
}
void drawPr2Temp(char value[3]){
  if(strcmp(lastTempPR2, value)==0)
    return;
  strcpy(lastTempPR2,value);
  tft.fillRect(textDataStart, 96, 12, 10, ST7735_BLACK);
  tft.setCursor(textDataStart, 96);
  tft.print(value);
}
void drawPr2Hum(char value[3]){
  if(strcmp(lastHumPR2, value)==0)
    return;
  strcpy(lastHumPR2,value);
  tft.fillRect(textDataStart, 106, 12, 10, ST7735_BLACK);
  tft.setCursor(textDataStart, 106);
  tft.print(value);
}
void drawGarageTemp(char value[3]){
  if(strcmp(lastTempGAR, value)==0)
    return;
  strcpy(lastTempGAR,value);
  tft.fillRect(textDataStart, 136, 12, 10, ST7735_BLACK);
  tft.setCursor(textDataStart, 136);
  tft.print(value);
}
void drawGarageHum(char value[3]){
  if(strcmp(lastHumGAR, value)==0)
    return;
  strcpy(lastHumGAR,value);
  tft.fillRect(textDataStart, 146, 12, 10, ST7735_BLACK);
  tft.setCursor(textDataStart, 146);
  tft.print(value);
}
