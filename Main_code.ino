#define BLYNK_TEMPLATE_ID "TMPLLKKDe96S"
#define BLYNK_TEMPLATE_NAME "Intelligent Hydroponics"
#define BLYNK_AUTH_TOKEN "LsnVHD63CAEbiD_e1wL6zd0xezPbJrBR"
#define BLYNK_PRINT Serial
#define ESP8266_BAUD 115200

#include <ESP8266_Lib.h>
#include <ezButton.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <SoftwareSerial.h>
#include <DHT.h>
SoftwareSerial EspSerial(12,13);

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "4GWIFI_69976";
char pass[] = "12345678";

ESP8266 wifi(&EspSerial);
BlynkTimer timer;


//motor pin defines
const int stepPin = 8;
const int dirPin = 9;
ezButton limitSwitch_down(10);

int Light_pin = 3;
int Pump_pin = 4;
int Oxygen_Pump_pin = 5;


//sensors defines
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define Liquid_Detection_Pin 27
int Liquid_detection_val;


#define TDS_pin A2

int TDS_read;
double TDS_DUMB;
double TDS_Value;

#define PH_pin A3
int PH_read;
double PH_Value;

#define flowPin 25
double flowRate;

volatile int count;
int limit = 1;



void sendSensor() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Blynk.virtualWrite(V23, 1);
  Serial.println("led High");
  delay(2000);
  Blynk.virtualWrite(V23, 0);
  Serial.println("led low");

  limitSwitch_down.loop();
  limit = limitSwitch_down.getState();
  if(limit==0){
    Blynk.virtualWrite(V4, TDS_Value);
    TDS_read = analogRead(TDS_pin);
    TDS_DUMB = 0.00147928 * (TDS_read) * (TDS_read)-0.600 * (TDS_read) + 244.854;

    if (TDS_DUMB > 100) {
      TDS_Value = TDS_DUMB;
    }

    PH_read = analogRead(PH_pin);
    float volt=((float)PH_read)/1024; //convert the analog into millivolt

    PH_Value = -20.00*volt + 19.80;
    
    Blynk.virtualWrite(V4, TDS_Value);
    // Serial.print("tds: ");
    // Serial.print(TDS_read);
    // Serial.print(", ");
    // Serial.println(TDS_Value);
    Blynk.virtualWrite(V5, PH_Value);
    // Serial.print("ph ");
    // Serial.print(PH_read);
    // Serial.print(", ");
    // Serial.println(PH_Value);
    

  }
  



  count = 0;
  interrupts();
  delay(1000);
  noInterrupts();
  flowRate = 12.972*(count);

  flowRate = flowRate * 60;
  flowRate = flowRate / 1000;

  Blynk.virtualWrite(V1, h);
  Serial.print("flow ");
  Serial.println(flowRate);

  Blynk.virtualWrite(V2, t); 
  Blynk.virtualWrite(V6, flowRate);
}

void setup() {
  Serial.begin(9600);
  EspSerial.begin(ESP8266_BAUD);
  delay(10);
  pinMode(stepPin,OUTPUT);
  pinMode(dirPin,OUTPUT);
  limitSwitch_down.setDebounceTime(50);
  pinMode(Light_pin, OUTPUT);
  pinMode(Pump_pin,OUTPUT);
  pinMode(Oxygen_Pump_pin,OUTPUT);
  

  dht.begin();
  pinMode(Liquid_Detection_Pin, INPUT);
  pinMode(TDS_pin, INPUT);
  pinMode(PH_pin, INPUT);
  pinMode(flowPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(flowPin), Flow, RISING);
  Blynk.begin(auth, wifi, ssid, pass);


  timer.setInterval(1000, sendSensor);

}



BLYNK_WRITE(V0){
  digitalWrite(Pump_pin, LOW);
  limitSwitch_down.loop();
  limit = limitSwitch_down.getState();
  int isData = 0;
  while (!isData){
    limitSwitch_down.loop();
    limit = limitSwitch_down.getState();
    if (limit == LOW){
      // Serial.println(limit);
      float avgTDS = 0;
      float avgPH = 0;
      for(int i=0; i<60; i++){
        TDS_read = analogRead(TDS_pin);
        TDS_DUMB = 0.00147928 * (TDS_read) * (TDS_read)-0.600 * (TDS_read) + 244.854;
        avgTDS += TDS_DUMB;
        if (TDS_DUMB > 100) {
          TDS_Value = TDS_DUMB;
        }

        PH_read = analogRead(PH_pin);
        float volt=((float)PH_read)/1024; //convert the analog into millivolt
        PH_Value = -20.00*volt + 19.80;
        avgPH += PH_Value;

        Blynk.virtualWrite(V4, avgTDS/(i+1));
        Blynk.virtualWrite(V5, avgPH/(i+1));
        delay(100);
      }
      Blynk.virtualWrite(V4, avgTDS/60);
      Blynk.virtualWrite(V5, avgPH/60);
      isData = 1;
      digitalWrite(dirPin, LOW); 
      for (int x = 0; x < 5000; x++){ 
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(1000);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(2);
      }
      
      limitSwitch_down.loop();
      limit = limitSwitch_down.getState();
      
    }
 else{
      digitalWrite(dirPin, HIGH);
      while(limit == HIGH){
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(1000);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(2);
        limitSwitch_down.loop();
        limit = limitSwitch_down.getState();
      }
    }
    digitalWrite(Pump_pin, HIGH);
    Blynk.virtualWrite(V0,0);
  }


}



BLYNK_WRITE(V8)
 {
 if(param.asInt()==1)
  {
     digitalWrite(Oxygen_Pump_pin, HIGH);
  }
  else
  {
     digitalWrite(Oxygen_Pump_pin, LOW);
  }
 }

BLYNK_WRITE(V7)
 {
 if(param.asInt()==1)
  {
     digitalWrite(Pump_pin, HIGH);
  }
  else
  {
     digitalWrite(Pump_pin, LOW);
  }
 }


BLYNK_WRITE(V9)
 {
 if(param.asInt()==1)
  {
     digitalWrite(Light_pin, HIGH);
  }
  else
  {
     digitalWrite(Light_pin, LOW);
  }
 }

void loop()
{
  Blynk.run();

  timer.run();

}

void Flow() {
  count++;
}
