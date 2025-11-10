#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "local_config.h"

EthernetClient client;
PubSubClient pclient(client);

// alfa= (2pi*fc)/fa considerando fa= 10kHz e fc= 10Hz temos alfa= 0.006283
// considerando a freq de Nyquist fs/2= 5kHz filtro rc para atenuar isso
// fc para o filtro 159Hz 0.006283f
// C= 100nF e R= 10kOhm

#define alvo 6.283185307179586476925286766559e-4f
unsigned long tempo_ultimo= 0;
unsigned long tempo_atual= 0;
const long tempo_amostra= 1020; 

float f1 = 0.0;
float f2 = 0.0;

unsigned ant_raw= 0;
float tensao= 0;
float ma= 0;
float corrente= 0;

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

  Ethernet.init(PA4);

  Serial.begin(115200);
  Serial.println("Iniciando");
  pclient.setServer(mqttServer, mqttPort);
  pclient.setKeepAlive(20);

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

  pinMode(PA1, INPUT);
  pinMode(PC13, OUTPUT);
  //LED feedback
  digitalWrite(PC13, LOW);

  // coleta inicial do filtro
  ant_raw= analogRead(PA1);
  f1= (float)ant_raw;
  f2= (float)ant_raw;
  delay(200);
}

void loop() {

  if (!pclient.connected()) {
    reconnect();
  }
  pclient.loop();

  ant_raw = analogRead(PA1);

  f1= (1.0f-alvo)*f1+(alvo*(float)ant_raw);
  f2= (1.0f-alvo)*f2+(alvo*f1);

  tempo_atual= millis();
  
  if(tempo_atual-tempo_ultimo>=tempo_amostra){
    tempo_ultimo= tempo_atual;

    tensao= (0.00320297894364437*f2)+0.0541041383627242; 
    // Serial.println(tensao, 2);
    ma= (tensao/97.10)*1000;
    corrente= (ma*6.25)-75; 
    // Serial.println(corrente, 2);   

    char jsonBuffer[256]; 
    int len = snprintf(jsonBuffer, sizeof(jsonBuffer), "{ \"tensao_medida\": %.4f, \"valor_analogico\": %.4f, \"corrente_medida\": %.4f }",tensao, f2, corrente);

    if (len < 0 || len >= sizeof(jsonBuffer)) {
      Serial.println("ERRO: Buffer JSON muito pequeno ou falha na formatação.");
    }
    if (pclient.publish(mqttTopic, jsonBuffer)) {
      Serial.print("Publicado: ");
      Serial.println(jsonBuffer);
    } else {
      Serial.print("Falha ao publicar. Estado: ");
      Serial.println(pclient.state());
    }
  }
}