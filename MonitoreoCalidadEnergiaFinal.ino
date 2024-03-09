/*******************************************************************  
*  Firmware final Sistemas Inteligentes II                          *
*                                                                   *
*  Por:                                                             *  
*  - Juan Pablo Osuna Bonilla                                       *
*  - Geradeyra Acevedo Sierra                                       *
*  - Christian Cardona Sánchez                                      *
*  - Laura Daniela Cruz Ducuara                                     *
********************************************************************/

/**************************************************************
*                          LIBRERIAS                          *
***************************************************************/
#include <WiFi.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>
#include <math.h>


/**************************************************************
*               CREDENCIALES COMUNICACIÓN WI-FI               *
***************************************************************/
const char* ssid = "ProyectosElectronica";
const char* password = "proyectos";
const char* mqtt_server = "broker.emqx.io";


/**************************************************************
*               CONSTANTES, VARIABLES Y OBJETOS               *
***************************************************************/
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE    (110)
char msg[MSG_BUFFER_SIZE];
int value = 0;
PZEM004Tv30 pzem(Serial2, 16, 17);


/**************************************************************
*                 FUNCIÓN CONFIGURACION WI-FI                 *      
***************************************************************/
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Se inicializa con la credenciales
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


/**************************************************************
*                     FUNCIÓN CALLBACK                        *      
***************************************************************/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  S+erial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  /*Condicional para enviar un HIGH o LOW al pin de control del módulo relé
   *para accionar el bombillo, con respecto al valor del tópico que ingresa*/
  if ((char)payload[0] == '1') {
    digitalWrite(18, HIGH);  
  } else {
    digitalWrite(18, LOW);  // Turn the LED off by making the voltage HIGH
  }
}


/**************************************************************
*                  FUNCIÓN RECONEXIÓN WI-FI                   *      
***************************************************************/
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      //Suscripción al tópico "inTopic"
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


/**************************************************************
*                       FUNCIÓN SETUP                         *      
***************************************************************/
void setup()
  //Se configura el pin de control del módulo relé (18) como salida.
  pinMode(18, OUTPUT);
  Serial.begin(115200);
  //Se configura el WiFi
  setup_wifi();
  //Se configura el servidor para mqtt en el puerto 1883
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


/**************************************************************
*                       FUNCIÓN LOOP                          *      
***************************************************************/
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //Se calculan los parámetros de la energía mediante el sensor PZEM-004T
  float voltaje = pzem.voltage();
  float corriente = pzem.current();
  float potencia = pzem.power();
  float energia = pzem.energy();
  float frecuencia = pzem.frequency();
  float factorpotencia = pzem.pf();
  float pot_aparente = voltaje*corriente;
  float pot_activa = pot_aparente*factorpotencia;
  float pot_reactiva = sqrtf(pow(pot_aparente, 2) - pow(pot_activa, 2));

  if (!isnan(voltaje) && !isnan(corriente) && !isnan(potencia) && !isnan(energia) && !isnan(frecuencia) && !isnan(factorpotencia) && !isnan(pot_activa) && !isnan(pot_reactiva) && !isnan(pot_aparente)) {
    snprintf (msg, MSG_BUFFER_SIZE, "V: %.2fV, C: %.2fA, P: %.2fW, E: %.3fkWh, F: %.1fHz, PF: %.2f, PAC: %.2fW, PRAC: %.2fVAR, PAP: %.2fVA ", voltaje, corriente, potencia, energia, frecuencia, factorpotencia, pot_activa, pot_reactiva, pot_aparebte);
    snprintf (msg, MSG_BUFFER_SIZE, "P: %.2fW", potencia);
    Serial.print("Ms: ");
    Serial.println(msg);
    //Conversión de los parámetros calculados a String
    String voltage = String(voltaje);
    String current = String(corriente);
    String power = String(potencia);
    String energy = String(energia);
    String frequency = String(frecuencia);
    String pfactor = String(factorpotencia);
    String pactive = String(pot_activa);
    String preactive = String(pot_reactiva);
    String papparent = String(pot_aparente);
    //Envío de los datos al cliente MQTT mediante sus respectivos tópicos
    client.publish("/voltaje", voltage.c_str());
    client.publish("/corriente", current.c_str());
    client.publish("/potencia", power.c_str());
    client.publish("/energia", energy.c_str());
    client.publish("/frecuencia", frequency.c_str());
    client.publish("/factorpotencia", pfactor.c_str());
    client.publish("/pot_activa", pactive.c_str());
    client.publish("/pot_reactiva", preactive.c_str());
    client.publish("/pot_aparente", papparent.c_str());
  }
  delay(5000);
}