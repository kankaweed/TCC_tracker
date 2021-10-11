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
 * CSV File
 * https://scholarslab.lib.virginia.edu/blog/saving-arduino-sensor-data/
 * 
*/

/*
 * LDR's
 * https://www.filipeflop.com/universidade/kit-maker-arduino/projeto-10-sensor-de-luz-ambiente/
*/
#include <dht11.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define DHT11PIN 9
#define AMOSTRAS 12
#define TTracker A8
#define TFixo A9
#define TMotor A10
#define DS1307_ADDRESS 0x68

byte zero = 0x00; //workaround for issue #527

float humidity, temperature, voltageTracker, voltageFixed, voltageMotor, currentTracker, currentFixed, currentMotor;

// valor máximo de tensão na entrada do arduino
float aRef = 5;

// Relação calculada para o divisor de tensão
float relation = 12;

int sensitivity = 66, adcValue= 0, offsetVoltage = 2500;

const int revolution15Degress = 80000; // Trocar valor para baseado na implementação
const int revolution20Degress = 80000; // Trocar valor para baseado na implementação
const int revolution30Degress = 80000; // Trocar valor para baseado na implementação
const int revolution50Degress = 80000; // Trocar valor para baseado na implementação
const int revolutionBack = 10000000;

const int stepPin = 7;
const int dirPin = 8;

const int limitSwitch1 = 2;
const int limitSwitch2 = 3;

boolean safetyStop = false;

const int sdPin = 4;

File dataFile;

int sensorLeftBot = A12;
int sensorLeftTop = A15;
int sensorRightBot = A13;
int sensorRightTop = A14;

int valueLeftBot = 0;
int valueLeftTop = 0;
int valueRightBot = 0;
int valueRightTop = 0;

dht11 DHT11;

void setup(){
  Serial.begin(9600);  

  Serial.print("Inicializando o cartão SD...");
  if(!SD.begin(sdPin)) {
    Serial.println("Falha na inicialização!");
  }
  Serial.println("Inicialização feita com sucesso!.");
  
  dataFile = SD.open("data.csv", FILE_WRITE);
  
  if (dataFile) {
    dataFile.println("hora,umidade,temperatura,tensao_tracker,tensao_fixo,tensao_motor,corrente_tracker,corrente_fixed,corrente_motor");
    dataFile.close();
    Serial.println("Arquivo aberto com sucesso");
  } else {
    Serial.println("Falha ao abrir o arquivo");
  }

  Wire.begin();
 
  pinMode(limitSwitch1, INPUT_PULLUP);
  pinMode(limitSwitch2, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(limitSwitch1), stopMotor, CHANGE);
  attachInterrupt(digitalPinToInterrupt(limitSwitch2), stopMotor, CHANGE);

  pinMode( stepPin, OUTPUT ) ;
  pinMode( dirPin, OUTPUT ) ;
  digitalWrite(stepPin, LOW);
  
  // descomentar a linha àbaixo somente se for necessário regravar o horário no RTC
  //setDateTime();
}

void loop(){
  String completeDate = returnCompleteDate();
  readSensors();

  voltageTracker = readVoltage(TTracker);
  voltageFixed = readVoltage(TFixo);
  voltageMotor = readVoltage(TMotor);

  currentTracker = readCurrentSensor(A3);
  currentFixed = readCurrentSensor(A4);
  currentMotor = readCurrentSensor(A5);

  if(returnHourAndMinute() == "08:30"){
    turnMotor(revolution15Degress, HIGH);
  }
   if(returnHourAndMinute() == "10:00"){
    turnMotor(revolution15Degress, HIGH);
  }
   if(returnHourAndMinute() == "11:30"){
    turnMotor(revolution20Degress, HIGH);
  }
   if(returnHourAndMinute() == "13:00"){
    turnMotor(revolution50Degress, HIGH);
  }
   if(returnHourAndMinute() == "14:30"){
    turnMotor(revolution30Degress, HIGH);
  }
   if(returnHourAndMinute() == "16:30"){
    turnMotor(revolutionBack, LOW);
  }
  
  /*Serial.print("Data: ");
  Serial.println(completeDate);
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
  Serial.println("A");*/

  saveDataToFile();
  //Serial.println();
  // aumentar delay para medições quando for instalado
  delay(5000);
}

void readSensors(){
  DHT11.read(DHT11PIN);
  
  humidity = DHT11.humidity;
  temperature = DHT11.temperature;
  
  /*Serial.print("Umidade: ");
  Serial.print(humidity, 2);
  Serial.println(" %");
  
  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" ºC");*/
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
  int adjustDirection = 0;

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
  digitalWrite(stepPin, LOW);
  
  int valueLeftBot = analogRead(sensorLeftBot);
  int valueLeftTop = analogRead(sensorLeftTop);
  int valueRightBot = analogRead(sensorRightBot);
  int valueRightTop = analogRead(sensorRightTop);

  adjustDirection = calcLDRDifference();

  if(adjustDirection == 1) {
    turnMotor(1000, HIGH);
  } else if(adjustDirection == 2) {
    turnMotor(1000, LOW);
  }
}

int calcLDRDifference() {
  int averageRight = valueRightBot + valueRightTop;
  int averageLeft = valueLeftBot + valueLeftTop;
  

  int difference = averageRight - averageLeft;

  if(difference >= 100) {
    return 1;
  }
  else if(difference <= -100) {
    return 2;
  }
  
  return 0;
}

void stopMotor() {
  safetyStop = true;
}

void saveDataToFile(){
  dataFile = SD.open("data.csv", FILE_WRITE);
  dataFile.println( 
                    String(returnCompleteDate()) + "," +
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


void setDateTime(){
  // As seguinte variaveis servem para definir a data e o horário que será gravado no RTC
  byte second =      0; //0-59
  byte minute =      05; //0-59
  byte hour =        14; //0-23
  byte weekDay =     7; //1-7
  byte monthDay =    9; //1-31
  byte month =       10; //1-12
  byte year  =       21; //0-99

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator

  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(weekDay));
  Wire.write(decToBcd(monthDay));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));

  Wire.write(zero); //start 

  Wire.endTransmission();

}

byte decToBcd(byte val){
// Converte números decimais para BCD
  return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val)  {
// Converte BCD para números decimais
  return ( (val/16*10) + (val%16) );
}

String returnCompleteDate(){

  // Reinicia o ponteiro do registrador
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  int second = bcdToDec(Wire.read());
  int minute = bcdToDec(Wire.read());
  int hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  int weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  int monthDay = bcdToDec(Wire.read());
  int month = bcdToDec(Wire.read());
  int year = bcdToDec(Wire.read());

  //Concatena a data no seguinte formato: 30/04/1997 23:30:00
  String completeDate = String(monthDay) + "/" + String(month) + "/" + String(year) + " " + String(hour) + ":" + String(minute) + ":" + String(second);
  
  return completeDate;

}

String returnHourAndMinute(){

  // Reinicia o ponteiro do registrador
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  int second = bcdToDec(Wire.read());
  int minute = bcdToDec(Wire.read());
  int hour = bcdToDec(Wire.read() & 0b111111);

  //retorna a hora e  ominuto no seguinte formato 23:59
  String hourAndMinute = String(hour) + ":" + String(minute);
  
  return hourAndMinute;

}
