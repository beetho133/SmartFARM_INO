#include <SoftwareSerial.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Tx (보내는 핀 설정)
int blueTx = 12;
// Rx (받는 핀 설정)
int blueRx = 13;
#define DHTPIN 2//내부온습도
#define LIGHT_P A0//조도
#define SOIL_HUMI A1//토양습도
#define SOIL_TEMP 5//토양온도
#define PUMP 8 // 펌프
#define HUMIDIFIER 9 // 가습
#define HEATER 10 // 발열
#define COOLER 11 // 환풍

#define DHTTYPE DHT11

SoftwareSerial myBT(blueTx, blueRx);
OneWire ourWire(SOIL_TEMP);
DallasTemperature sensors(&ourWire);
DHT dht(DHTPIN, DHTTYPE);
//0x27, 0x3F
LiquidCrystal_I2C lcd(0x3F, 16, 2);

//데이터를 수신 받을 버퍼
byte buffer[512];
char sensorData[512];
//버퍼에 데이터를 저장할 때 기록할 위치
int bufferPosition;
int sensorPosition;

//센서 관련 변수 초기화
int light = 0;
int innerTemp = 0;
int innerHumi = 0;
int soilTemp = 0;
int soilHumi = 0;

int pu_act = 0;
int hu_act = 0;
int he_act = 0;
int co_act = 0;

//LCD관련 변수
int light_p;
String lcdLine1;
String lcdLine2;
int tmpLength = 100;


// 온도 조건
int  min_innerTemp = 25;
int  mid_innerTemp = 27;
int  max_innerTemp = 30;

int  min_innerHumi = 45;
int  mid_innerHumi = 50;
int  max_innerHumi = 55;

int  min_soilTemp = 25;
int  mid_soilTemp = 27;
int  max_soilTemp = 30;

int  min_soilHumi = 45;
int  mid_soilHumi = 50;
int  max_soilHumi = 55;

void setup() {
  Serial.begin(9600);

  myBT.begin(9600);

  Serial.println("ATcommand");

  pinMode(SOIL_HUMI, INPUT);
  pinMode(SOIL_TEMP, INPUT);

  pinMode(PUMP, OUTPUT);
  pinMode(HUMIDIFIER, OUTPUT);
  pinMode(HEATER, OUTPUT);
  pinMode(COOLER, OUTPUT);

  sensors.begin();

  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();

  digitalWrite(PUMP, HIGH);
  digitalWrite(HUMIDIFIER, HIGH);
  digitalWrite(HEATER, HIGH);
  digitalWrite(COOLER, HIGH);

  //버퍼 위치 초기화
  bufferPosition = 0;
  sensorPosition = 0;
}

//조도 센서 데이터
void getLight() {
  int tempLight = analogRead(A0);  
  light = map(tempLight, 0, 1023, 2000, 0);
  lcdLine1 += (String)light;
}

//기온, 습도 센서 데이터
void getInnerTemp_Humi() {
  innerTemp = dht.readTemperature();
  innerHumi = dht.readHumidity();
  
  if (isnan(innerHumi) || isnan(innerTemp)) {
    Serial.println("Failed to read from DHT!!");
  }
  lcdLine1 += (String)innerTemp;
  lcdLine1 += "C | ";
  lcdLine1 += (String)innerHumi;
  lcdLine1 += "% | ";
}

//토양 센서 데이터
void getSoilTemp_Humi() {
  sensors.requestTemperatures();
  //토양온도
  soilTemp = sensors.getTempCByIndex(0);
  //토양습도
  int value = analogRead(SOIL_HUMI);

  soilHumi = map(value, 400, 1023, 100, 50);

  lcdLine2 += (String)soilTemp;
  lcdLine2 += "C | ";
  lcdLine2 += (String)soilHumi;
  lcdLine2 += "% | ";
}

void loop() {
    
  lcdLine1 = "";
  lcdLine2 = "";

  //시리얼 모니터 내부 출력
  //Serial.println("센서 데이터");
  getInnerTemp_Humi();
  getSoilTemp_Humi();
  getLight();
   
  lcd.setCursor(0, 0);
  lcd.print(lcdLine1);
  lcd.setCursor(0, 1);
  lcd.print(lcdLine2);

  if(innerTemp < -30 || innerTemp > 50 || innerHumi < 0 || innerHumi > 100
    || soilTemp < -30 || soilTemp > 50 || innerHumi < 0 || innerHumi > 100) {
      return;
  }
  
  ////// 센서값 받기/////
  String tmp = (String)light+","+(String)innerTemp+","+(String)innerHumi+","+(String)soilTemp
  +","+(String)soilHumi+","+(String)pu_act+","+(String)hu_act+","+(String)he_act+","+(String)co_act;

  //펌프
  if(soilHumi < min_soilHumi){
    digitalWrite(PUMP, LOW); // 펌프 켬
    pu_act = 1;
  }
  if(soilHumi >= mid_soilHumi){
    digitalWrite(PUMP, HIGH); // 펌프 끔
    pu_act = 0;
  }
      
  //발열기
  if(innerTemp < min_innerTemp || soilTemp < min_soilTemp){
    digitalWrite(HEATER, LOW); // 발열 켬
    he_act = 1;
  }
  if(innerTemp >= mid_innerTemp && soilTemp >= mid_soilTemp){
    digitalWrite(HEATER, HIGH); // 발열 끔
    he_act = 0;
  }
        
  //환풍기
  if(innerTemp > max_innerTemp || soilTemp > max_soilTemp || innerHumi > max_innerHumi || soilHumi > max_soilHumi) {
    if(pu_act == 0 && he_act == 0 && hu_act == 0) {
      digitalWrite(COOLER, LOW); // 환풍 켬
      co_act = 1;
    }
  }
  if(innerTemp <= mid_innerTemp && soilTemp <= mid_soilTemp && innerHumi <= mid_innerHumi && soilHumi <= mid_soilHumi) {
    digitalWrite(COOLER, HIGH); // 환풍 끔
    co_act = 0;
  }
        
  //가습기
  if(innerHumi < min_innerHumi){
    digitalWrite(HUMIDIFIER, LOW); // 가습 켬
    hu_act = 1;
  }
  if(innerHumi >= mid_innerHumi){
    digitalWrite(HUMIDIFIER, HIGH); // 가습 끔
    hu_act = 0;
  }
  
  myBT.println(tmp);
  //myBT.println(light+innerTemp+innerHumi+soilTemp+soilHumi);
  if (myBT.available()) {
    // 수신 받은 데이터 저장
    byte data = myBT.read();
    // 수신 받은 데이터를 버퍼에 저장
    buffer[bufferPosition++] = data;

    // 문자열 종료 표시
    if (data == '\n') {
      buffer[bufferPosition] = '\0';
    String result = "";

    for(int i=0; i<bufferPosition-1; i++){
      result+= (char)buffer[i];
    }
    Serial.print("버튼 값 : ");
    Serial.println(result);
      bufferPosition = 0;

        //펌프
        if(result.equals("pon")){
         digitalWrite(PUMP, LOW); // 펌프 켬
         pu_act = 1;
        }else if(result.equals("poff")){
         digitalWrite(PUMP, HIGH); // 펌프 끔
         pu_act = 0;
        }
        
        //발열기
        if(result.equals("fon")){
         digitalWrite(HEATER, LOW); // 발열 켬
         he_act = 1;
        }else if(result.equals("foff")){
         digitalWrite(HEATER, HIGH); // 발열 끔
         he_act = 0;
        }
        
        //환풍기
        if(result.equals("von")){
         digitalWrite(COOLER, LOW); // 환풍 켬
         co_act = 1;
        }else if(result.equals("voff")){
         digitalWrite(COOLER, HIGH); // 환풍 끔
         co_act = 0;
        }
        
        //가습기
        if(result.equals("hon")){
         digitalWrite(HUMIDIFIER, LOW); // 가습 켬
         hu_act = 1;
        }else if(result.equals("hoff")){
         digitalWrite(HUMIDIFIER, HIGH); // 가습 끔
         hu_act = 0;
        } 
    }
  }
 //delay(5000);
}
