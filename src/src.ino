/* 
 *  DHT11
 *  https://create.arduino.cc/projecthub/arcaegecengiz/using-dht11-b0f365
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
#define DS1307_ADDRESS 0x68 // pino hexademcima para RTC

byte zero = 0x00;

float humidity, temperature, voltageTracker, voltageFixed, voltageMotor, currentTracker, currentFixed, currentMotor;

// valor máximo de tensão na entrada do arduino
float aRef = 5;

// Relação calculada para o divisor de tensão
float relation = 12;

int sensitivity = 66, adcValue= 0, offsetVoltage = 2500;

const unsigned long leaveLimitSwitch1 = 10000; // solta fim de curso
const unsigned long revolution2Position = 128270; // posição 2
const unsigned long revolution3Position = 168918; // posição 3
const unsigned long revolution4Position = 54957; // posição 4
const unsigned long revolution5Position = 49497; // posição 5
const unsigned long revolution6Position = 113639; // posição 6
const unsigned long revolution7Position = 81425; // posição 7
const unsigned long arriveLimitSwitch2 = 10000; // bate fim de curso 2
const unsigned long leaveLimitSwitch2 = 10000; // solta fim de curso 2
const unsigned long restPosition = 256427; // posição de descanço
const unsigned long arriveLimitSwitch1= 400000; // posição inicial

const int stepPin = 7;
const int dirPin = 8;
const int enableMotor = 10;

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
    dataFile.println("hora,umidade,temperatura,tensao_tracker,tensao_fixo,tensao_motor,corrente_tracker,corrente_fixed,corrente_motor,pot_tracker,pot_fixed");
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
  digitalWrite(enableMotor, HIGH);  

  Serial.println("Digite os números das posições");
  // descomentar a linha abaixo somente se for necessário regravar o horário no RTC
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

  // teste usando a Serial

  /*if(Serial.available() > 0){
    int numero = 10;
    numero = Serial.read();
    
    Serial.print(numero);
    
    if(numero == '1'){
      turnMotor(revolution1Position, HIGH);
      numero = 10;
    }
    if(numero == '2'){
      turnMotor(revolution2Position, HIGH);
      numero = 10;
    }
    if(numero == '3'){
      turnMotor(revolution3Position, HIGH);
      numero = 10;
    }
    if(numero == '4'){
      turnMotor(revolution4Position, HIGH);
      numero = 10;
    }
    if(numero == '5'){
      turnMotor(revolution5Position, HIGH);
      numero = 10;
    }
    if(numero == '6'){
      turnMotor(revolution6Position, HIGH);
      numero = 10;
    }
    if(numero == '7'){
      turnMotor(revolution7Position, HIGH);
      numero = 10;
    }
    if(numero == '8'){
      turnMotor(revolution8Position, LOW);
      numero = 10;
    }
    if(numero == '9'){
      turnMotor(revolution9Position, LOW);
      numero = 10;
    }
    if(numero == '0'){
      turnMotor(andaTudo, HIGH);
      numero = 10;
    }
  }*/
  
  if(returnHourAndMinute() == "7:00"){
    turnMotor(arriveLimitSwitch1, LOW);
    delay(1000);
    turnMotor(leaveLimitSwitch1, HIGH);
  }
  if(returnHourAndMinute() == "8:30"){
   turnMotor(revolution2Position, HIGH);
  }
  if(returnHourAndMinute() == "10:00"){
     turnMotor(revolution3Position, HIGH);
  }
  if(returnHourAndMinute() == "12:00"){
    turnMotor(revolution4Position, HIGH);
  }
  if(returnHourAndMinute() == "13:00"){
   turnMotor(revolution5Position, HIGH);
  }
  if(returnHourAndMinute() == "14:30"){
     turnMotor(revolution6Position, HIGH);
  }
  if(returnHourAndMinute() == "16:00"){
    turnMotor(revolution7Position, HIGH);
  }
  if(returnHourAndMinute() == "18:00"){
    turnMotor(arriveLimitSwitch2, HIGH);
    delay(1000);
    turnMotor(leaveLimitSwitch2, HIGH);
    delay(1000);
    turnMotor(restPosition, LOW);
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
  Serial.println();
  // aumentar delay para medições quando for instalado
  //delay(5000);
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

void turnMotor(unsigned long motorStep, volatile byte dir ) {
  digitalWrite(dirPin, dir);
  digitalWrite(enableMotor, LOW);
  int adjustDirection = 0;
  Serial.print("motorStep: ");
  Serial.println(motorStep);

  for(int x = 0; x < motorStep; x++) {
    if(safetyStop){
      safetyStop = false;
      /*digitalWrite(dirPin, !dir);
      
      for(int i = 0; i < limitSwitchStep; i++){
        digitalWrite(stepPin, HIGH); 
        delayMicroseconds(500); 
        digitalWrite(stepPin, LOW); 
        delayMicroseconds(500);   
      }*/
      
      digitalWrite(enableMotor, HIGH);
      Serial.print("steps: ");
      Serial.println(x);
      break;
    }
    digitalWrite(stepPin, HIGH); 
    delayMicroseconds(500); 
    digitalWrite(stepPin, LOW); 
    delayMicroseconds(500);
  }
  digitalWrite(dirPin,!dir); 
  digitalWrite(stepPin, LOW);
  
  valueLeftBot = analogRead(sensorLeftBot);
  valueLeftTop = analogRead(sensorLeftTop);
  valueRightBot = analogRead(sensorRightBot);
  valueRightTop = analogRead(sensorRightTop);

  adjustDirection = calcLDRDifference();

  if(adjustDirection == 1) {
    turnMotor(1000, HIGH);
  } else if(adjustDirection == 2) {
    turnMotor(1000, LOW);
  }
  digitalWrite(enableMotor, HIGH);
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
  Serial.println("Entrei na interrupção");
}

void saveDataToFile(){
  if (returnMinute() == "00" || returnMinute() == "15" || returnMinute() == "30" || returnMinute() == "45"){
    float potTracker = voltageTracker * currentTracker;
    float potFixed = voltageFixed * currentFixed;
    float potMotor = voltageMotor * currentMotor;
    
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
                      String(currentMotor) + "," +
                      String(potTracker) + "," +
                      String(potFixed) + "," +
                      String(potMotor)
                    );
    dataFile.close();
  }
}


void setDateTime(){
  // As seguinte variaveis servem para definir a data e o horário que será gravado no RTC
  byte second =      0; //0-59
  byte minute =      33; //0-59
  byte hour =        15; //0-23
  byte weekDay =     1; //1-7
  byte monthDay =    17; //1-31
  byte month =       10; //1-12
  byte year  =       21; //0-99

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //parar Oscilador

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

  //Concatena a data no seguinte formato: 30/04/97 23:30:00
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

String returnMinute(){

  // Reinicia o ponteiro do registrador
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  int second = bcdToDec(Wire.read());
  int minute = bcdToDec(Wire.read());

  //retorna a hora e  ominuto no seguinte formato 23:59
  String onlyMinute = String(minute);
  
  return onlyMinute;

}
