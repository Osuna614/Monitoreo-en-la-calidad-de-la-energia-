/*******************************************************************  
*  Firmware final Sistemas Inteligentes II                          *
*                                                                   *
*  Por:                                                             *  
*  - Juan Pablo Osuna Bonilla                                       *
*  - Geradeyra Acevedo Sierra                                       *
*  - Christian Cardona Sánchez                                      *
*  - Laura Daniela Cruz Ducuara                                     *
********************************************************************/


/*Se incluyen las librería necesarias para el programa*/
#include <PubSubClient.h>
#include <PZEM004Tv30.h>
#include <math.h>
 
/*Definición de constantes, variables y objetos que se van a utilizar en el programa*/
int value = 0;
PZEM004Tv30 pzem(Serial2, 16, 17);

/**************************************************************
*                       FUNCIÓN SETUP                         *      
***************************************************************/

void setup()
  //Se configura el pin de control del módulo relé (18) como salida.
  pinMode(18, OUTPUT);
  Serial.begin(115200);
}

/**************************************************************
*                       FUNCIÓN LOOP                          *      
***************************************************************/

void loop() {
  client.loop();
  //Se calculan los parámetros de la energía mediante el sensor PZEM-004T
  float voltage = pzem.voltage(); //Voltaje
  float current = pzem.current(); //Corriente 
  float power = pzem.power(); //Potencia
  float energy = pzem.energy(); //Energia
  float frequency = pzem.frequency(); //Frecuencia
  float pf = pzem.pf(); //FactordePotencia
  float pot_apa = voltage*current;  //PotenciaAparente
  float pot_act = pot_apa*pf; //PotenciaActiva
  float pot_rea = sqrtf(pow(pot_apa, 2) - pow(pot_act, 2)); /PotenciaReactiva
 
  delay(5000);
}