
/*Mini weather station
Pedro Blasco pblamar925@g.educaand.es
*/
#include <PubSubClient.h> //MQTT Client
#include "DHT.h" //Sensor humedad
#include <Wire.h> //Pantalla
#include "SSD1306.h" //Pantalla
#include <NTPClient.h> //Servidor hora
#include <ESP8266WiFi.h> //WIfi
#include <WiFiUdp.h> //UDP para servidor hora

//Defino Sensor de humedad
#define DHTTYPE DHT11 
DHT dht(12,DHTTYPE); //12 -->D6)

//Defino pantalla
SSD1306 display(0x3c, 5, 4); //SSD1306 display(0x3c, SDA_PIN, SCK_PIN)
//GPIO 5 = D1, GPIO 4 = D2
#define flipDisplay true
#define OLED_RESET    -1 //No hay pin reset en la pantalla

//************** WIFI CONF **************
const char *ssid     = "CasaBM1";
const char *password = "hr5-jk@27HJyure76";
const char* mqtt_server = "192.168.0.19";
//************** NTP CONF ***************
const long utcOffsetInSeconds = 3600; //UTC+1
char daysOfTheWeek[7][12] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//************** MQTT CONF *************
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//Recibe mensajes MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Intento conectar
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado MQTT");
     
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  display.init();
  if (flipDisplay) display.flipScreenVertically();
  
  /* Mostramos la pantalla de bienvenida */
  display.clear();
  display.init();
  display.setContrast(255);
  display.setFont(ArialMT_Plain_16);
  display.drawString(15, 25, "¡¡Hola, Pedro!! ");
  display.display();
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  timeClient.begin();
  dht.begin();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
//*********************************************
void loop() {
  //Actualizo la hora
  timeClient.update();
  //Leo temperaturas y humedad
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  //Presento en pantalla
  display.clear();
  display.drawLine(0,0,DISPLAY_WIDTH,0);
  display.drawRect(0,15,DISPLAY_WIDTH, DISPLAY_HEIGHT-16);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0,2,"Proto1:MiniWeather Station");
  display.drawString(3,15,"Temperatura: "+ String(t) + "º");
  display.drawString(3,30,"Humedad: " + String(h) + "%");
  display.drawString(3,50,"Última lectura: " + timeClient.getFormattedTime());
  display.display();
  //Envío por MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
 
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    Serial.print("Publico temperatura a : ");
    Serial.print(String(timeClient.getFormattedTime()));
    Serial.println(t);
    client.publish("casa/salon/temperatura", String(t).c_str(), true);
    client.publish("casa/salon/humedad", String(h).c_str(),true);
  }

}
