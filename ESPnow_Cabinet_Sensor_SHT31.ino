#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

#define SCL 16
#define SDA 18

char myMac[18];

uint8_t receiverAddress[] = {0x48, 0x27, 0xE2, 0x46, 0xC1, 0xEC};

typedef struct struct_message {
  char mac[18];
  int temperature;
  int humidity;
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Failed");
}

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

Adafruit_SHT31 sensor = Adafruit_SHT31();

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }
  WiFi.mode(WIFI_STA);

  WiFi.macAddress().toCharArray(myMac, 18);

  if(esp_now_init() != ESP_OK){
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if(esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  Wire.begin(SDA, SCL);
  Serial.println("Initializing sensor");
  initSensor();
}

void loop() {
  Serial.println("Loop!");

  int temp = (int)sensor.readTemperature();
  int hum = (int)sensor.readHumidity();
  
  Serial.print("Sending Data: MAC->");
  Serial.print(myMac);
  Serial.print("   Temp->");
  Serial.print(temp);
  Serial.print("   Hum->");
  Serial.println(hum);

  strcpy(myData.mac, myMac);
  myData.temperature = temp;
  myData.humidity = hum;

  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *) &myData, sizeof(myData));

  if(result == ESP_OK){
    Serial.println("Sending Confirmed");
  } else {
    Serial.println("Seding Error");
  }

  lastTime = millis();
  
  delay(5000);
}

void initSensor(){
  if (!sensor.begin()) {
    Serial.println("Couldn't find filaments sensor, check wiring!");
    while (1);
  } else {
    Serial.println("Sensor Initialized");
  }
}