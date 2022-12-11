/*
Sketch Nodo Final MQTT
Sensores utilizados:  temperatura y humedad DHT22,radiacion UV ML8511UV, humedad del suelo YL-100, temperatura del suelo DS1820, iluminancia TEMT6000
Numero de NODO 1234  
Desarrollado por ING JULIAN CAMACHO
Para Servicio Nacional de Aprendizaje SENA
Version 8.0
24/10/2022
TEST ok 
*/
//Librerias utilizadas en este Sketch 

#include <DHT.h>      // para sensor DHT22
#include <SPI.h>   
#include <RH_RF95.h>  //libreria modulo radio LoRa
#include <string.h>   //libreria para habilitar funciones con cadenas
#include <Arduino.h>
#include <math.h>     //libreria para desarrollo matematico
#include <OneWire.h>  //libreria para utilizar el protocolo one wire con el sensor DS1820             
#include <DallasTemperature.h> //libreria para el sensor de temperatura DS1820
#include "LowPower.h"

#define DHTPIN 3       //pin digital para la comunicacion del sensor DHT22
#define DHTTYPE DHT22  //se crea una instancia para el sensosr DHT22
#define light A4       //entrada analoga para el sensor de luz 

DHT dht(DHTPIN, DHTTYPE); //crea una isntancia del controlador DHT 
RH_RF95 rf95;             //crea una instancia del controlador de Radio
OneWire ourWire(4);       //Se establece el pin 2  como bus OneWire para el sensor de temperatura DS1820 //pullup resistor 4.7k
DallasTemperature sensors(&ourWire); //Se declara un objeto para el sensor DS1820

float frequency = 915.0;  //frequencia de funcionamiento modulos LoRa
float uvIntensity;        //variable para el sensor de UV
unsigned int count = 1;   //contador de transmisiones
int ReadUVintensityPin = A0; //entrada analoga para el sensor UV
int SensorPin = A1;      //entrada analoga sensor de humedad de suelo

char buffertint[] = "\0";  //variable para separacion de enteros 
char buffertdec[] = "\0";  //variables para separacion de decimales
char bufferhint[] = "\0";
char bufferhdec[] = "\0"; 
char bufferuvint[] = "\0"; 
char bufferuvdec[] = "\0";   
String message =""; // mensaje a ser enviado


void setup()
{
     dht.begin();        //se inicia el sensor de T y HR DHT22
     sensors.begin();   //Se inicia el sensor de temperatura DS1820
     Serial.begin(9600);  // se inicia el monitor serial
     Serial.println(F("Start MQTT NODE"));
          if (!rf95.init())    //error al inicial el modulo RF
     Serial.println(F("init failed"));
         rf95.setFrequency(frequency);   //configuracion del modulo RF
         rf95.setTxPower(13);
         rf95.setSyncWord(0x34);  
     pinMode(ReadUVintensityPin, INPUT);
     pinMode(SensorPin, INPUT);
     pinMode(light, INPUT);
}
// funcion para calcular el promdeio de lecturas del pin analogo
int averageAnalogRead(int pinToRead)  
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;

  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return(runningValue);
}

//funcion Map de arduno para variables tipo float //From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void loop()
{
    Serial.print("###########    ");
    Serial.print("COUNT=");
    Serial.print(count);
    Serial.println("    ###########");
///////////////////////////DHT22 SENSOR////////////////////////////////////////////////////
      // Leemos la humedad relativa
      float hum = dht.readHumidity();    //entera mas decimal
    
      // Leemos la temperatura en grados centígrados (por defecto)
      float tem = dht.readTemperature();
    
      // Comprobamos si las lecturas pueden dar algún fallo mediante la función isnan() Esta función devuelve un 1 en caso de que el valor no sea numérico
      if (isnan(hum) || isnan(tem)) {
        Serial.println("Error obteniendo los datos del sensor DHT11");
        return;
      }
///////////////////////////ML8511 SENSOR////////////////////////////////////////////////////////

      int uvLevel = averageAnalogRead(ReadUVintensityPin);
      float outputVoltage = 5.0 * uvLevel/1024;
      uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);   //se ajusta a la hoja de datos del sensor
      //uvIntensity = ((int) (uvIntensity*10))/10.0;         // dejar a un solo decimal

/////////////////////////Moisture SENSOR///////////////////////////////////////////////////////
      int m = 0;
      m = analogRead(SensorPin);
      m = map(m, 0, 1023, 100, 0);   //ajuste 0-100% de la humedad

////////////////////////SOIL TEMPERATURE SENSOR///////////////////////////////////////////////////////
      sensors.requestTemperatures();   //Se envía el comando para leer la temperatura
      float tempsoil= sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC 
 
/////////////////////////Light SENSOR///////////////////////////////////////////////////////

      //int Lvalue = analogRead(light);// read the light
      //int lux = map(Lvalue,0, 1023, 0, 100); 

      float volts = analogRead(light) * 5.0 / 1024.0;
      float amps = volts / 10000.0; // across 10,000 Ohms
      float microamps = amps * 1000000;
      float lux = microamps * 2.0;

/////////////////////////Mostrar variables en la terminal//////////////////////////////////////////////
           
       Serial.print(F("The temperature and humidity:"));
       Serial.print(tem);
       Serial.print("℃");
       Serial.print(",");
       Serial.print(hum);
       Serial.print("%");
       Serial.println("");
       Serial.print("UV Intensity: ");
       Serial.print(uvIntensity);
       Serial.print("mW/cm^2");
       Serial.println();
       Serial.print("Soil Moisture: ");
       Serial.print(m); 
       Serial.println("% ");
       Serial.print("Soil Temperature: ");
       Serial.print(tempsoil);
       Serial.println("℃");
       Serial.print("Light Intensity: ");
       Serial.print(lux);//
       Serial.print("% ");
       Serial.println();    
       
       delay(500);
       
   
      float temperature = tem*10;      //para obtener un solo decimal multiplico por diez
      float rounded_temperature = round(temperature);  //redondeo y quito la parte decimal
      rounded_temperature/=10;         //obtengo el decimal redondeado
      int tint=rounded_temperature;        // obtengo la parte entera
      //sprintf(buffertint, formato, tint);
      float tres=rounded_temperature-tint;
      int tdec=tres*10;                    // obtengo la parte decimal
    
      float humidity = hum*10;
      float rounded_humidity = round(humidity);
      rounded_humidity/=10; 
      int hint=rounded_humidity;
      //sprintf(bufferhint, formato, hint);
      float hres=rounded_humidity-hint;
      int hdec=hres*10;
      //sprintf(bufferhdec, formato, hdec);
    
      float uv = uvIntensity*10;
      float rounded_uv = round(uv);
      rounded_uv/=10; 
      int uvint=rounded_uv;
      //sprintf(bufferhint, formato, hint);
      float uvres=rounded_uv-uvint;
      int uvdec=uvres*10;
    
      float tsoil = tempsoil*10;
      float rounded_tsoil = round(tsoil);
      rounded_tsoil/=10; 
      int tsoilint=rounded_tsoil;
      //sprintf(bufferhint, formato, hint);
      float tsoilres=rounded_tsoil-tsoilint;
      int tsoildec=tsoilres*10;

      //se arma la cadena que va a ser enviada al gateway
    
      message="<1234>"; //Node ID - cambiar para cada nodo final.
      message+="field1=";
      message+=tint;
      message+=".";
      message+=tdec;
      message+="&field2=";
      message+=hint;
      message+=".";
      message+=hdec;
      message+="&field3=";
      message+=uvint;
      message+=".";
      message+=uvdec;
      message+="&field4=";
      message+=m;
      message+="&field5=";
      message+=tsoilint;
      message+=".";
      message+=tsoildec;
      message+="&field6=";
      message+=lux;
    
      char data[76] = "\0";    //el tamaño depende del numero de caracteres
      
        
      message.toCharArray(data,76);  //convierto string a arreglo tipo char y lo almaceno en la variable data
    
      uint8_t datasend[76];
    
      strcpy(datasend,data);   //copio la variable data a la variable tipo uint8_t llamada datasend
      
       //Serial.println((char *)datasend);
      Serial.print("size ");
      Serial.println(sizeof datasend);
        //memset(datatemp,"",9*sizeof(char));
    
      message ="";  
          
      delay(1000);
      
          Serial.println(F("Sending data to LG01"));
          
          rf95.send((char *)datasend,sizeof(datasend));     //envio los datos al gateway
          rf95.waitPacketSent();  // Now wait for a reply
        
          uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
          uint8_t len = sizeof(buf);
    
       if (rf95.waitAvailableTimeout(3000))
       { 
        // Should be a reply message for us now   
        if (rf95.recv(buf, &len))                        //espero respuesta del gateway
       {
         
          Serial.print("got reply from LG01: ");
          Serial.println((char*)buf);
          Serial.print("RSSI: ");
          Serial.println(rf95.lastRssi(), DEC);    
        }
        else
        {
          Serial.println("recv failed");
        }
      }
      else
      {
        Serial.println("No reply, is LoRa server running?");
      }
      count++;
      //delay(60000);
      for (int i = 0 ;  i  <  16 ; i++)                   //activo el modo ahorro de energia
                 LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);  //envio datos cada minuto
     
}
    
