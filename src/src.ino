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
 * 
*/

/*
 * CSV File
 * https://scholarslab.lib.virginia.edu/blog/saving-arduino-sensor-data/
 * 
*/
#include <dht11.h>
#include <DS1307.h>
#include <SPI.h>
#include <SD.h>

#define DHT11PIN 9
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

const int sdPin = 4;

File dataFile;

dht11 DHT11;

void setup(){
  Serial.begin(9600);  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Inicializando o cartão SD...");
  if(!SD.begin(sdPin)) {
    Serial.println("Falha na inicialização!");
    return;
  }
  Serial.println("Inicialização feita com sucesso!.");
  
  dataFile = SD.open("data.csv", FILE_WRITE);
  
  if (dataFile) {
    dataFile.println("hora,umidade,temperature,tensao_tracker,tensao_fixo,tensao_motor,corrente_tracker,corrente_fixed,corrente_motor");
    dataFile.close();
    Serial.println("Arquivo aberto com sucesso");
  } else {
    Serial.println("Falha ao abrir o arquivo");
  }
  
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
  
}

void loop(){
  readSensors();

  voltageTracker = readVoltage(A0);
  voltageFixed = readVoltage(A1);
  voltageMotor = readVoltage(A2);

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
  Serial.print(voltageTracker);
  Serial.println("V");
  Serial.print("Tensão fixo: ");
  Serial.print(voltageFixed);
  Serial.println("V");
  Serial.print("Tensão motor: ");
  Serial.print(voltageMotor);
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

  saveDataToFile();
  Serial.println();
  delay(5000);
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
  float total = 0, calc;
  for(int i = 0; i < AMOSTRAS; i++) {
    total += 1.0 * analogRead(ioPin);
    delay(5);
  }
  calc = (total / (float)AMOSTRAS) * relation;

  return (calc * aRef) / 1024.0;
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

void saveDataToFile(){
  dataFile = SD.open("data.csv", FILE_WRITE);
  //dataFile.print("umidade,temperature,tensao_tracker,tensao_fixo,tensao_motor,corrente_tracker,corrente_fixed,corrente_motor");
  //voltageTracker, voltageFixed, voltageMotor, currentTracker, currentFixed, currentMotor
  dataFile.println( String(rtc.getTimeStr()) + "," +
                    String(humidity) + "," + 
                    String(temperature) + "," + 
                    String(voltageTracker) + "," +
                    String(voltageFixed) + "," + 
                    String(voltageMotor) + "," + 
                    String(currentTracker) + "," + 
                    String(currentFixed) + "," +
                    String(currentMotor)
                  );
  dataFile.close();
}
