#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "internet.h"
#include <ezTime.h>

#define led 2


const char *mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_client_id = "senai134_esp_giovanna";
const char *mqtt_topic_sub = "shrek123/pub";
const char *mqtt_topic_pub = "burro123/pub";

void conectaMqtt();
void retornoMqtt(char *topic, byte *payload, unsigned int lenght);

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

    Serial.print("enviado: ");
    Serial.println(stringJson);

    
  }
  
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

void retornoMqtt(char *topic, byte *payload, unsigned int lenght){
  Serial.print("Mensagem recebida em ");
  Serial.print(topic);
  Serial.print(": ");

  String msgRecebida = "";
  for (int i = 0; i < lenght; i++)
  {
    msgRecebida += (char)payload[i];
  }

  Serial.println(msgRecebida);

  if (msgRecebida == "L" || msgRecebida == "l"){
    digitalWrite(led, HIGH);
  } else if ( msgRecebida == "d" || msgRecebida == "D"){
    digitalWrite(led, LOW);
  }

  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, msgRecebida);


  if (erro)
  {
    Serial.print("Erro ao decodificar o JSON: ");
    Serial.println(erro.c_str());
    return;
  }

  const char* dispositivo = doc["disp"];
  const char* timestamp = doc["timestamp"];
  const char* msg = doc["msg"];
  Serial.println("Dados decodificados");
  Serial.print("Esp: ");
  Serial.println(dispositivo);
  Serial.print("Data e hora: ");
  Serial.println(timestamp);
  Serial.print("Mensagem: ");
  Serial.println(msg);
  Serial.println("--------------------------------");

  
String mensagemTexto = String(msg); // Converte para String para poder pesquisar palavras
  mensagemTexto.toLowerCase();        // Deixa tudo minúsculo 
  if (mensagemTexto.indexOf("desligar") != -1)
  {
    digitalWrite(led, LOW);
  }
  else if (mensagemTexto.indexOf("ligar") != -1)
  {
    digitalWrite(led, HIGH);
  }
    
  
  
}