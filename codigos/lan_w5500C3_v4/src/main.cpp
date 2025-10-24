#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "local_config.h"

EthernetClient client;
PubSubClient pclient(client);

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexao MQTT...");
    
    if (pclient.connect(clientId, mqttUser, mqttPassword)) { 
      Serial.println(" Conectado!");
    } else {
      Serial.print(" Falha, rc=");
      Serial.print(pclient.state());
      Serial.println(". Tentando novamente em 0,5 segundos.");
      delay(500);
    }
  }
}

void setup() {

  Ethernet.init(7);

  Serial.begin(115200);
  // while (!Serial) {
  //   ; // Apenas para teste
  // }
  Serial.println("Iniciando");
  pclient.setServer(mqttServer, mqttPort);

  // podemos usar o Ethernet.begin(mac, ip, eth_DNS, gateway, subnet);
  Ethernet.begin(mac);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Dispositivo não encotrado. :(");
    while (true) {
      delay(1); 
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Cabo não conectado.");
  }

  Serial.print("Micro IP: ");
  Serial.println(Ethernet.localIP());

  delay(200);
  pinMode(1, INPUT);
  pinMode(8, OUTPUT);
  //LED feedback
  digitalWrite(8, LOW);
}

void loop() {

  if (!pclient.connected()) {
    reconnect();
  }
  pclient.loop();

  int sensorReading = analogRead(1);
  float tensao = (3.3 / 4095.0) * sensorReading; // sem correcao * 0.766665
  float corrente = (62.5 * tensao) - 75; 

  char jsonBuffer[256]; 
  int len = snprintf(jsonBuffer, sizeof(jsonBuffer), "{ \"tensao_medida\": %.4f, \"valor_analogico\": %d, \"corrente_medida\": %.4f }",tensao, sensorReading, corrente);

  if (len < 0 || len >= sizeof(jsonBuffer)) {
    Serial.println("ERRO: Buffer JSON muito pequeno ou falha na formatação.");
  }

  // String jsonData = "{";
  // jsonData += "\"tensao_medida\": " + String(tensao, 4) + ", ";
  // jsonData += "\"valor_analogico\": " + String(sensorReading) + ", ";
  // jsonData += "\"corrente_medida\": " + String(corrente, 4);
  // jsonData += "}";

  if (pclient.publish(mqttTopic, jsonBuffer)) {
    Serial.print("Publicado: ");
    Serial.println(jsonBuffer);
  } else {
    Serial.print("Falha ao publicar. Estado: ");
    Serial.println(pclient.state());
  }

  //evita sobrecarregar a rede, 900ms para tentar bater os 1hz com margem 
  delay(900); 
}