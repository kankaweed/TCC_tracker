/* 
 *  DHT11
 *  https://create.arduino.cc/projecthub/arcaegecengiz/using-dht11-b0f365
*/

/* 
 *  Anemometro
 *  calc:https://webstorage.cienciaviva.pt/public/pt.cienciaviva.io/recursos/files/anemometro_4943648725bffc.pdf
 *  conections: http://blog.baudaeletronica.com.br/sensor-hall-com-arduino/
*/

/* 
 *  
 * Sensor de tensão
 * https://br-arduino.org/2015/06/voltimetro-com-arduino-como-montar-programar-e-calibrar.html
 *  
*/

/*
 * Sesor de corrente
 * https://www.electronicshub.org/interfacing-acs712-current-sensor-with-arduino/
 * 
*/

/*
 * Motor
 * https://www.robocore.net/tutoriais/controlando-motor-de-passo
 * 
*/

/*
 * RTC
 * https://www.filipeflop.com/blog/relogio-rtc-ds1307-arduino/
 * https://github.com/filipeflop/DS1307
*/
#include <dht11.h>
#include <DS1307.h>

#define DHT11PIN 4
#define AMOSTRAS 12

DS1307 rtc(5, 6);

float humidity, temperature, voltageTracker, voltageFixed, voltageMotor, currentTracker, currentFixed, currentMotor;

// valor máximo de tensão na entrada do arduino
float aRef = 5;

// Relação calculada para o divisor de tensão
float relation = 12;

int sensitivity = 66, adcValue= 0, offsetVoltage = 2500;

const int revolution15Degress = 200; // Trocar valor para baseado na implementação
const int revolution20Degress = 200; // Trocar valor para baseado na implementação
const int revolution30Degress = 200; // Trocar valor para baseado na implementação
const int revolution50Degress = 200; // Trocar valor para baseado na implementação
const int revolutionBack = 10000000;

const int stepPin = 7;
const int dirPin = 8;

const int limitSwitch1 = 2;
const int limitSwitch2 = 3;

boolean safetyStop = false;

dht11 DHT11;

void setup(){
  rtc.halt(false);
  
  //Deve ser colocado no código para a primeira ativação do RTC, então comnetado e recarregado
  rtc.setDOW(SATURDAY);      //Define o dia da semana
  rtc.setTime(15, 16, 0);     //Define o horario
  rtc.setDate(02, 10, 2021);   //Define o dia, mes e ano

  rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);
  
  pinMode(limitSwitch1, INPUT_PULLUP);
  pinMode(limitSwitch2, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(limitSwitch1), stopMotor, CHANGE);
  attachInterrupt(digitalPinToInterrupt(limitSwitch2), stopMotor, CHANGE);

  pinMode ( stepPin, OUTPUT ) ;
  pinMode ( dirPin, OUTPUT ) ;
  Serial.begin(9600);  
}

void loop(){
  readSensors();

  voltageTracker = (readVoltage(A0) * aRef) / 1024.0;
  voltageFixed = (readVoltage(A1) * aRef) / 1024.0;
  voltageMotor = (readVoltage(A2) * aRef) / 1024.0;

  currentTracker = readCurrentSensor(A3);
  currentFixed = readCurrentSensor(A4);
  currentMotor = readCurrentSensor(A5);
  
  if(rtc.getTimeStr(FORMAT_SHORT) == "08:30"){
    turnMotor(revolution15Degress, HIGH);
  }
   if(rtc.getTimeStr(FORMAT_SHORT) == "10:00"){
    turnMotor(revolution15Degress, HIGH);
  }
   if(rtc.getTimeStr(FORMAT_SHORT) == "11:30"){
    turnMotor(revolution20Degress, HIGH);
  }
   if(rtc.getTimeStr(FORMAT_SHORT) == "13:00"){
    turnMotor(revolution50Degress, HIGH);
  }
   if(rtc.getTimeStr(FORMAT_SHORT) == "14:30"){
    turnMotor(revolution30Degress, HIGH);
  }
   if(rtc.getTimeStr(FORMAT_SHORT) == "08:30"){
    turnMotor(revolutionBack, LOW);
  }

  Serial.print("Tensão tracker: ");
  Serial.print(voltageTracker * relation);
  Serial.println("V");
  Serial.print("Tensão fixo: ");
  Serial.print(voltageFixed * relation);
  Serial.println("V");
  Serial.print("Tensão motor: ");
  Serial.print(voltageMotor * relation);
  Serial.println("V");

  Serial.print("Corrente tracker: ");
  Serial.print(currentTracker);
  Serial.println("A");
  Serial.print("Corrente fixed: ");
  Serial.print(currentFixed);
  Serial.println("A");
  Serial.print("Corrente motor: ");
  Serial.print(currentMotor);
  Serial.println("A");
 
  Serial.println();
  delay(2000);
}

void readSensors(){
  DHT11.read(DHT11PIN);
  
  humidity = DHT11.humidity;
  temperature = DHT11.temperature;
  
  Serial.print("Umidade: ");
  Serial.print(humidity, 2);
  Serial.println(" %");
  
  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" ºC");
}

float readVoltage(uint8_t ioPin) {
  float total = 0;
  for(int i = 0; i < AMOSTRAS; i++) {
    total += 1.0 * analogRead(ioPin);
    delay(5);
  }
  return total / (float)AMOSTRAS;
}

float readCurrentSensor(uint8_t ioPin) {
  double adcVoltage = 0;
  
  adcValue = analogRead(ioPin);
  adcVoltage = (adcValue / 1024.0) * 5000;
  
  return ((adcVoltage - offsetVoltage) / sensitivity);
}

void turnMotor(int motorStep, volatile byte dir ) {
  digitalWrite(dirPin, dir);

   for(int x = 0; x < motorStep; x++) {
    if(safetyStop){
      safetyStop = false;
      break;
    }
        digitalWrite(stepPin, HIGH); 
        delayMicroseconds(500); 
        digitalWrite(stepPin, LOW); 
        delayMicroseconds(500); 
    }
   digitalWrite(dirPin,!dir); 
}

void stopMotor() {
  safetyStop = true;
}
