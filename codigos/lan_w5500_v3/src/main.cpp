#include <Arduino.h>
#include <SPI.h>
#include <EthernetESP32.h>
#include "local_config.h"

int server_port = 8000; 
EthernetClient client;
const unsigned long CONNECT_TIMEOUT_MS = 5000; 

bool connectWithTimeout(EthernetClient& cli, IPAddress ip, int port, unsigned long timeout) {
    if (cli.connected()) {
        cli.stop();
    }
    cli.connect(ip, port); 

    unsigned long startTime = millis();
    while (!cli.connected() && (millis() - startTime) < timeout) {
        delay(10); 
    }

    if (cli.connected()) {
        return true;
    } else {
        cli.stop(); 
        return false;
    }
}

void setup() {

  // Pino CS escolhido
  Ethernet.init(5);

  Serial.begin(115200);
  // while (!Serial) {
  //   ; // Apenas para teste
  // }
  Serial.println("Iniciando");

  // Adicionar DNS e gateway para conexão ,eth_DNS, gateway
  Ethernet.begin(mac, ip, eth_DNS, gateway, subnet);

  // Checagem de Hardware
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Dispositivo não encotrado. :(");
    while (true) {
      delay(1); 
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Cabo não conectado.");
  }

  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  Serial.print("Gateway: ");
  Serial.println(Ethernet.gatewayIP());

  Serial.print("DNS: ");
  Serial.println(Ethernet.dnsServerIP());

  delay(200); // Espera um pouco para a rede estabilizar

  //Pino para medição
  pinMode(15, INPUT);
}

void loop() {

  int sensorReading = analogRead(15);
  float tensao = (3.3 / 4095.0) * sensorReading * 0.766665;
  float corrente = (39.5265 * tensao) - 50;

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

  Serial.print("Conectando ao servidor em ");
  Serial.print(server_ip);
  Serial.print("...");

  if (connectWithTimeout(client, server_ip, server_port, CONNECT_TIMEOUT_MS)) {
    Serial.println(" Conectado!");
    
    client.setTimeout(1000);
    
    client.println("POST / HTTP/1.1");
    client.print("Host: ");
    client.println(server_ip);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(len); 
    client.println(); 
    client.print(jsonBuffer); 
    
    // while (client.available()) {
    //   char c = client.read();
    //   Serial.print(c);
    // }
    client.stop();
    Serial.println("\nConexao fechada.");
    delay(100);

  } else {
    Serial.println(" Conexao falhou!");
    client.stop(); 
    //ESP.restart();
  }
}