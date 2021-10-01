# TCC_tracker

## DHT11
---
- Foi utilizado a biblioteca dht11 para facilitar a leitura da umidade e temperatura;
- O sensor dht11 está sendo lido no pino **digital** 4;
- A leitura do sensor está dentro da função **readSensors()** que está sendo chamada dentro do loop;
### Esquemático
![Esquemático DHT11](./images/esquematico_dht11.png )

**NÃO COLOCAR RESISTOR** para o dht11 que só possui 3 terminais

## Sensor de Tensão
---
- Foi utilizado um divisor de tensão que permite ler uma tensão de até 60V;
- Para esse circuito foram utilizados um resistor de 220KΩ (R1) e um resistor de 20KΩ (R2);
- Os sensores de tensão estão conectados nas entradas:
    - A0: Tracker;
    - A1: Fixo;
    - A2: Motor;
- O valor da tensão é calculado pela função **readVoltage**
### Esquemático

![Esquemático divisor de tensão](./images/divisor_tensao.png )

## Sensor de corrente
---
- Foi utilizado o sensor ACS712-30A
- Os sensores de corrente estão conectados nas entradas:
    - A3: Tracker;
    - A4: Fixo;
    - A5: Motor;
- O valor da corrent é calculado pela função **readCurrentSensor**

### Esquemático
![Esquemático sensor de corrente](./images/sensor_corrente.png )

# Referências

- dht11: https://create.arduino.cc/projecthub/arcaegecengiz/using-dht11-b0f365

- Sensor de tensão: https://br-arduino.org/2015/06/voltimetro-com-arduino-como-montar-programar-e-calibrar.html

- Sensor de corrente: https://www.electronicshub.org/interfacing-acs712-current-sensor-with-arduino/