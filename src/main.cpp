#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "internet.h"
#include <ezTime.h>

#define led 2
unsigned long tempoAntPisca = 0;
unsigned long intervalo = 3000;
unsigned long tempoPressionou = 0;

bool estadoL = false;
bool piscando = false;
bool pressionado = false;
bool estadoRemoto = false;


const char *mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_client_id = "senai134_esp_giovanna";
const char *mqtt_topic_sub = "shrek123/pub";
const char *mqtt_topic_pub = "burro123/pub";

void conectaMqtt();
void retornoMqtt(char *topic, byte *payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(espClient);

Timezone sp;

void setup() {

  pinMode(led, OUTPUT);

  Serial.begin(115200);
  Serial.setTimeout(5000);

  conexaoWifi();

  waitForSync();

  sp.setLocation("America/Sao_Paulo");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(retornoMqtt);



}

void loop() {

  unsigned long agora = millis();

  events();
  reconexaoWifi();
  client.loop();

  time_t timestampLocal = sp.now();

  String dataFormatada = sp.dateTime(timestampLocal, "d/m/Y H:i:s");

  if (!client.connected())
  {
    conectaMqtt();
  }

  if(Serial.available() > 0){
    String textoDigitado = Serial.readStringUntil('\n');
    textoDigitado.trim();

    if (textoDigitado.length() == 0)
    {
      Serial.println("Msg vazia");
      return;
    }

    JsonDocument doc;
    doc["timestamp"] = dataFormatada;
    doc["disp"] = "Esp-giovanna";
    doc["msg"] = textoDigitado;
    

    String stringJson;
    serializeJson(doc, stringJson);

    client.publish(mqtt_topic_pub, stringJson.c_str());

    if (textoDigitado == "L")
    {
      piscando = false;
      estadoL = true;
    }
    else if ( textoDigitado == "D")
    {
      piscando = false;
      estadoL = false;
    }
    else if (textoDigitado == "P")
    {
      piscando = true;
    }
    else
    {
      Serial.println("Mensagem invalida");
    }

    Serial.print("enviado: ");
    Serial.println(stringJson);

    
  }

  if (piscando){
    if (agora - tempoAntPisca >= intervalo){
    estadoL = !estadoL;
    tempoAntPisca = agora;
  }
  
  }
  digitalWrite(led, estadoL);
}



void conectaMqtt(){
  while (!client.connected())
  {
    Serial.println("Conectando ao MQTT ");

    if (client.connect(mqtt_client_id))
    {
      Serial.println("Conectado");
      client.subscribe(mqtt_topic_sub);
    }
    else{
      Serial.println("Falha ao se conectar ");
      Serial.println(client.state());
      Serial.println("Tentando novamente em 5s");
      delay(5000);

    }
    
  }
  
}

void retornoMqtt(char *topic, byte *payload, unsigned int length){
  Serial.print("Mensagem recebida em ");
  Serial.print(topic);
  Serial.print(": ");

  String msgRecebida = "";
  for (unsigned int i = 0; i < length; i++) {
    msgRecebida += (char)payload[i];
  }
  msgRecebida.trim();
  Serial.println(msgRecebida);

  String comando = msgRecebida;

  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, msgRecebida);

  if (!erro && doc["msg"].is<const char *>()) {
    const char *dispositivo = doc["disp"] | "desconhecido";
    const char *timestamp = doc["timestamp"] | "sem data";
    const char *msg = doc["msg"];

    Serial.println("Dados decodificados");
    Serial.print("Esp: ");
    Serial.println(dispositivo);
    Serial.print("Data e hora: ");
    Serial.println(timestamp);
    Serial.print("Mensagem: ");
    Serial.println(msg);
    Serial.println("--------------------------------");

    comando = String(msg);
    comando.trim();
  } else {
    Serial.println("Mensagem em texto puro ");
  }

  comando.toUpperCase();

  if (comando == "L") {
    piscando = false;
    estadoL = true;
  } else if (comando == "D") {
    piscando = false;
    estadoL = false;
  } else if (comando == "P") {
    piscando = true;
  } else {
    Serial.println("Comando invalido");
  }
}
