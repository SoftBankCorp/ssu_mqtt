#include <Wire.h>
#include <Adafruit_INA219.h>
#include <WiFi.h>
#include <string.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <mcp2515.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// BME280 주소 설정
#define BME280_ADDRESS 0x76
// 릴레이 핀 및 내장 LED 핀 설정
#define RELAY 16
#define LED_BUILTIN 2
#define can_id_1 0x0F6
#define CS_PIN 5  // Use GPIO5 for CS
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
const int motorPin = 26; 

// BME280 및 INA219 객체 생성
Adafruit_INA219 ina219;
Adafruit_BME280 bme;

struct can_frame canMsg1;
struct can_frame canMsg2;
int flag = 0;

// WiFi 및 MQTT 클라이언트 객체 생성
WiFiClient espClient;
PubSubClient client(espClient);

// CAN 통신을 위한 MCP2515 객체 생성 (핀 번호: 5)
MCP2515 mcp2515(CS_PIN);

// WiFi 및 MQTT 서버 정보
const char* ssid = "U+Net4670";
const char* password = "000J94#1D3";
const char* mqtt_server = "broker.mqtt-dashboard.com";

// 타이머 변수 및 샘플링 간격
unsigned long lastSampleTime = 0;
unsigned long lastRawSendTime = 0;  // raw 데이터를 5초마다 전송하기 위한 타이머
const long sampleInterval = 100;  // 100ms마다 샘플링
const long rawSendInterval = 5000; // 5초마다 raw 데이터를 전송

int sampleCount = 0;
float bmeTempData[10];  // BME280 온도 데이터 저장 배열
float bmeHumData[10];   // BME280 습도 데이터 저장 배열
float inaCurrentData[10]; // INA219 전류 데이터 저장 배열
float inaVoltageData[10]; // INA219 전압 데이터 저장 배열

float rawTempQueue[50];  // raw 데이터를 저장할 큐 (5초 동안의 데이터를 저장)
float rawHumQueue[50];
float rawCurrentQueue[50];
float rawVoltageQueue[50];
int rawQueueIndex = 0;  // 큐 인덱스

float tempSum = 0, humSum = 0, currentSum = 0, voltageSum = 0;


bool motorStatus = false;
int motorSpeed = 0;
int motorCurrentSpeed = 0; 
bool motorSlowDownRequested = false;


void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void queueRawData(float temp, float hum, float current, float voltage);
void sendRawDataQueueToMQTT();
void sendAverageDataToMQTT(float tempAvg, float humAvg, float currentAvg, float voltageAvg);
void float_to_int8_for_can(float tempAvg, float humAvg, float currentAvg, float voltageAvg);


TaskHandle_t TaskSensorReadHandle = NULL;
TaskHandle_t TaskMQTTHandle = NULL;
TaskHandle_t TaskCANHandle = NULL;
TaskHandle_t TaskMotorControlHandle = NULL;
TaskHandle_t TaskMotorSlowDownHandle = NULL;

void setup() {
  // 릴레이 및 내장 LED 핀 설정
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(motorPin, OUTPUT);

  pinMode(13, INPUT);  // E
  pinMode(27, INPUT);  // D
  pinMode(32, INPUT);  // P
  pinMode(33, INPUT);  // R
  pinMode(34, INPUT);  // wspeed (Analog input)

  // Initialize output pins
  pinMode(25, OUTPUT); // Motor control
  pinMode(12, OUTPUT); // Motor control
  pinMode(26, OUTPUT); // Motor PWM


  Serial.begin(115200); // 시리얼 통신 시작

  // CAN 통신 초기화
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CS_PIN);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();

  // INA219 센서 초기화
  ina219.begin();
  ina219.setCalibration_16V_400mA(); // 전류 및 전압 측정 범위 설정

  // BME280 센서 초기화
  if (!bme.begin(BME280_ADDRESS)) {
    Serial.println("Cannot find BME280");
    while (1); // BME280 센서를 찾을 수 없으면 무한 루프
  }
  Serial.println("BME280 sensor initialized.");

  // WiFi 및 MQTT 설정
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  canMsg1.can_id  = can_id_1;
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = 0x00;
  canMsg1.data[1] = 0x00;
  canMsg1.data[2] = 0x00;
  canMsg1.data[3] = 0x00;
  canMsg1.data[4] = 0x00;
  canMsg1.data[5] = 0x00;
  canMsg1.data[6] = 0x00;
  canMsg1.data[7] = 0x00;

 
  xTaskCreatePinnedToCore(
    TaskSensorRead,     
    "TaskSensorRead", 
    10000,             
    NULL,               
    1,                 
    &TaskSensorReadHandle,
    0);           

  xTaskCreatePinnedToCore(
    TaskMQTT,      
    "TaskMQTT",     
    10000,    
    NULL,       
    1,          
    &TaskMQTTHandle,Z
    1);

  xTaskCreatePinnedToCore(
    TaskCAN,          
    "TaskCAN",       
    10000,            
    NULL,               
    1,                  
    &TaskCANHandle,
    1);

  xTaskCreatePinnedToCore(
    TaskMotorControl,    
    "TaskMotorControl",  
    10000,               
    NULL,                
    1,                   
    &TaskMotorControlHandle,
    1);

  // Delete the setup and loop tasks to free up resources
  vTaskDelete(NULL);
}

void loop() {
  // Empty loop as tasks are running in FreeRTOS
}

// WiFi 연결 설정 함수
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // WiFi 연결 시작 (Station 모드)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // WiFi 연결 완료될 때까지 대기
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT 서버로부터 메시지 수신 시 호출되는 콜백 함수
void callback(char* topic, byte* payload, unsigned int length) {
  String jsonMessage = "";
  if (String(topic) == "motor/control") {
    for (int i = 0; i < length; i++) {
      jsonMessage += (char)payload[i];
    }
    Serial.println(jsonMessage);

    // Parse JSON
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, jsonMessage);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.f_str());
      return;
    }

    motorStatus = doc["status"];
    motorSpeed = doc["speed"].as<int>();
    int wspeed = motorSpeed;  
    int aspeed = map(wspeed, 0, 100, 0, 255);

    if (motorStatus) {
      digitalWrite(RELAY, HIGH);
      Serial.print("Motor ON with speed: ");
      Serial.println(motorSpeed);
      analogWrite(motorPin, aspeed);
      motorCurrentSpeed = aspeed;
    } else {
      Serial.println("Motor OFF");
      motorSlowDownRequested = true;
    }
  }
}

// MQTT 서버에 재연결하는 함수
void reconnect() {
  // MQTT 서버에 연결되지 않은 경우 연결 시도
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESPClient-" + String(random(0xffff), HEX);
    Serial.print("Client ID: ");
    Serial.println(clientId);

    // MQTT 서버에 연결 성공 시 구독 시작
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("BMS/power");
      client.subscribe("BMS/test");
      client.subscribe("BMS/current");
      client.subscribe("BMS/temp");
      client.subscribe("motor/control");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      vTaskDelay(5000 / portTICK_PERIOD_MS); // 연결 실패 시 5초 대기 후 재시도
    }
  }
}

// raw 데이터를 큐에 저장
void queueRawData(float temp, float hum, float current, float voltage) {
  if (rawQueueIndex < 50) {  // 5초 동안 100ms마다 50개의 데이터를 저장
    rawTempQueue[rawQueueIndex] = temp;
    rawHumQueue[rawQueueIndex] = hum;
    rawCurrentQueue[rawQueueIndex] = current;
    rawVoltageQueue[rawQueueIndex] = voltage;
    rawQueueIndex++;
  } else {
    Serial.println("Raw data queue is full");
  }
}

// 5초마다 큐에 저장된 raw 데이터를 한 번에 전송
void sendRawDataQueueToMQTT() {
  for (int i = 0; i < rawQueueIndex; i++) {
    byte dataPacket[16];  // 4바이트 x 4개 데이터 (온도, 습도, 전류, 전압)
    memcpy(&dataPacket[0], &rawTempQueue[i], sizeof(rawTempQueue[i]));
    memcpy(&dataPacket[4], &rawHumQueue[i], sizeof(rawHumQueue[i]));
    memcpy(&dataPacket[8], &rawCurrentQueue[i], sizeof(rawCurrentQueue[i]));
    memcpy(&dataPacket[12], &rawVoltageQueue[i], sizeof(rawVoltageQueue[i]));

    client.publish("BMS/raw", dataPacket, sizeof(dataPacket));  // MQTT로 패킷 전송
    Serial.print("Raw data queue sent: ");
    Serial.println(i);
  }
  Serial.println("Raw data queue sent to MQTT server");

  rawQueueIndex = 0;  // 전송 후 큐 초기화
}

// 평균 데이터를 MQTT 서버에 패킷 형식으로 전송하는 함수
void sendAverageDataToMQTT(float tempAvg, float humAvg, float currentAvg, float voltageAvg) {

  tempAvg = round(tempAvg);
  humAvg = round(humAvg);
  currentAvg = round(currentAvg);
  voltageAvg = round(voltageAvg);

  byte avgPacket[16];  // 4바이트 x 4개 데이터 (온도, 습도, 전류, 전압)
  memcpy(&avgPacket[0], &tempAvg, sizeof(tempAvg));
  memcpy(&avgPacket[4], &humAvg, sizeof(humAvg));
  memcpy(&avgPacket[8], &currentAvg, sizeof(currentAvg));
  memcpy(&avgPacket[12], &voltageAvg, sizeof(voltageAvg));

  client.publish("BMS/average", avgPacket, sizeof(avgPacket));  // MQTT로 패킷 전송
  Serial.println("Average data packet sent to MQTT server");
  for (int i = 0; i < 16; i++) {
    Serial.print(avgPacket[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void float_to_int8_for_can(float tempAvg, float humAvg, float currentAvg, float voltageAvg) {

  int8_t tempByte = (int8_t)(round(tempAvg));     
  int8_t humByte = (int8_t)(round(humAvg));        
  int8_t currentByte = (int8_t)(round(currentAvg));
  int8_t voltageByte = (int8_t)(round(voltageAvg));

  canMsg1.data[2] = tempByte;
  canMsg1.data[3] = humByte;
  canMsg1.data[4] = currentByte;
  canMsg1.data[5] = voltageByte;
}

void TaskSensorRead(void *pvParameters) {
  unsigned long lastSampleTime = millis();
  unsigned long lastRawSendTime = millis();

  for (;;) {
    unsigned long now = millis();

    // 100ms 간격으로 데이터를 수집
    if (now - lastSampleTime >= sampleInterval) {
      lastSampleTime = now;

      // 센서 데이터 수집 (BME280과 INA219)
      float v = ina219.getBusVoltage_V();
      float c = ina219.getCurrent_mA();
      float t = bme.readTemperature();
      float h = bme.readHumidity();

      if (c > 390) {
        Serial.println(c);
        Serial.println("overcurrent!:");
        Serial.println(c);
        digitalWrite(RELAY, LOW);
        analogWrite(26, 0);
        digitalWrite(25, LOW);
        digitalWrite(12, LOW);
        Serial.println("Emergency STOP");
      }

      // raw 데이터를 큐에 저장
      queueRawData(t, h, c, v);

      // 배열에 데이터 저장 및 합계 계산
      bmeTempData[sampleCount] = t;
      bmeHumData[sampleCount] = h;
      inaCurrentData[sampleCount] = c;
      inaVoltageData[sampleCount] = v;

      tempSum += t;
      humSum += h;
      currentSum += c;
      voltageSum += v;

      sampleCount++;

      // 1초(100ms * 10) 동안 데이터를 모은 후 평균값 전송
      if (sampleCount >= 10) {
        // 평균값 계산
        float tempAvg = tempSum / 10.0;
        float humAvg = humSum / 10.0;
        float currentAvg = currentSum / 10.0;
        float voltageAvg = voltageSum / 10.0;

        // 평균 데이터를 패킷 형식으로 MQTT 서버에 전송
        sendAverageDataToMQTT(tempAvg, humAvg, currentAvg, voltageAvg);
        float_to_int8_for_can(tempAvg, humAvg, currentAvg, voltageAvg);

        // 배열 및 합계 초기화
        sampleCount = 0;
        tempSum = 0;
        humSum = 0;
        currentSum = 0;
        voltageSum = 0;
      }
    }

    // 5초마다 raw 데이터를 한 번에 전송
    if (now - lastRawSendTime >= rawSendInterval) {
      lastRawSendTime = now;
      sendRawDataQueueToMQTT();  // 큐에 저장된 raw 데이터를 전송

      float v_a = ina219.getBusVoltage_V();
      float c_a = ina219.getCurrent_mA();
      Serial.print("V_a(voltageRAW):");
      Serial.println(v_a);
      Serial.print("c_a(currentRAW):");
      Serial.println(c_a);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); 
}


void TaskMQTT(void *pvParameters) {
  for (;;) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop(); // MQTT 통신을 처리

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}


void TaskCAN(void *pvParameters) {
  for (;;) {
    if (flag == 1) {
      canMsg1.data[0] = 0x01;
      canMsg1.data[1] = 0x00;
      mcp2515.sendMessage(&canMsg1);
      digitalWrite(LED_BUILTIN, HIGH);
    } else if (flag == 0) {
      canMsg1.data[0] = 0x00;
      canMsg1.data[1] = 0x01;
      mcp2515.sendMessage(&canMsg1);
      digitalWrite(LED_BUILTIN, LOW);
    }

    flag = !flag; // Toggle flag

    vTaskDelay(5000 / portTICK_PERIOD_MS); 
  }
}


void TaskMotorControl(void *pvParameters) {
  for (;;) {
    int D = digitalRead(27);
    int R = digitalRead(33);
    int P = digitalRead(32);
    int E = digitalRead(13);
    int wspeed = analogRead(34);
    int aspeed = map(wspeed, 0, 4095, 0, 255);

    if (aspeed >= 255) aspeed = 255;

    // 모터 동작 제어
    if (E == 1) {
      digitalWrite(RELAY, LOW);
      analogWrite(26, 0);
      digitalWrite(25, LOW);
      digitalWrite(12, LOW);
      Serial.println("Emergency STOP");
    } else if (P == 1 && E == 0) {
      digitalWrite(25, LOW);
      digitalWrite(12, LOW);
      analogWrite(26, 0);
    } else if (D == 1 && E == 0 && aspeed >= 40) {
      digitalWrite(25, HIGH);
      digitalWrite(12, LOW);f
      analogWrite(26, aspeed);
    } else if (R == 1 && E == 0 && aspeed >= 40) {
      digitalWrite(25, LOW);
      digitalWrite(12, HIGH);
      analogWrite(26, aspeed);
      Serial.println("Reverse engaged");
    }

    if (motorSlowDownRequested) {
      xTaskCreate(
        TaskMotorSlowDown,        
        "TaskMotorSlowDown", 
        2048,  
        NULL,  
        2, 
        &TaskMotorSlowDownHandle); 
      motorSlowDownRequested = false;
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskMotorSlowDown(void *pvParameters) {
  while (motorCurrentSpeed > 0) {
    motorCurrentSpeed -= 5;  
    if (motorCurrentSpeed < 0) motorCurrentSpeed = 0;
    analogWrite(motorPin, motorCurrentSpeed);
    Serial.print("Slowing down: Current speed = ");
    Serial.println(motorCurrentSpeed);
    vTaskDelay(100 / portTICK_PERIOD_MS); 
  }
  analogWrite(motorPin, 0); 
  Serial.println("Motor stopped");
  digitalWrite(RELAY, LOW);

  vTaskDelete(NULL);
}
