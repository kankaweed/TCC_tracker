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

//#define HALLPIN 3
//int hall,turns;
//float perimeter = 31.41, minuteInHour = 1667, vel = 0;

#include <dht11.h>

#define DHT11PIN 4
#define AMOSTRAS 12

float humidity, temperature, voltageTracker, voltageFixed, voltageMotor, currentTracker, currentFixed, currentMotor;

// valor máximo de tensão na entrada do arduino
float aRef = 5;

// Relação calculada para o divisor de tensão
float relation = 12;

int sensitivity = 66;
int adcValue= 0;
int offsetVoltage = 2500;
  
volatile byte state = LOW;

dht11 DHT11;

void setup(){
  Serial.begin(9600);  
}

void loop(){
  readSensors();
  
  voltageTracker = (readTrackerVoltage(A0) * aRef) / 1024.0;
  voltageFixed = (readTrackerVoltage(A1) * aRef) / 1024.0;
  voltageMotor = (readTrackerVoltage(A2) * aRef) / 1024.0;

  currentTracker = reandCurrentSensor(A3);
  currentFixed = reandCurrentSensor(A4);
  currentMotor = reandCurrentSensor(A5);

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

float readTrackerVoltage(uint8_t ioPin) {
  float total = 0;
  for(int i = 0; i < AMOSTRAS; i++) {
    total += 1.0 * analogRead(ioPin);
    delay(5);
  }
  return total / (float)AMOSTRAS;
}

float reandCurrentSensor(uint8_t ioPin) {
  double adcVoltage = 0;
  
  adcValue = analogRead(ioPin);
  adcVoltage = (adcValue / 1024.0) * 5000;
  
  return ((adcVoltage - offsetVoltage) / sensitivity);
}
