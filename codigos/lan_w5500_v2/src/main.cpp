#include <Arduino.h>
#include <SPI.h>
#include <EthernetESP32.h>
#include "local_config.h" 

// Carga para envio
const unsigned long TARGET_BYTES = 20UL * 1024UL * 1024UL; 

// Buffer
const int CHUNK_SIZE = 1460;
char dataChunk[CHUNK_SIZE];

// Inicia o servidor na porta 80
EthernetServer server(80);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Aguarda a porta serial conectar
  }

  Serial.println("\nTeste de Carga - Servidor Web Ethernet com ESP32");

  // Preenchimento do buffer
  memset(dataChunk, 'J', CHUNK_SIZE);

  Ethernet.init(5); // Pino 5 CS

  Serial.println("Iniciando conexao Ethernet...");
  //Ethernet.begin(mac, ip, eth_DNS, gateway);
  Ethernet.begin(mac, ip);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Shield Ethernet nao encontrado. Verifique as conexoes SPI e o pino CS.");
    while (true) delay(1);
  }
  
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Cabo de rede desconectado.");
  }

  delay(1000);

  // Inicia o servidor
  server.begin();
  Serial.print("Servidor iniciado. IP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Aguardando clientes para enviar ");
  Serial.print(TARGET_BYTES / 1024 / 1024);
  Serial.println(" MB de dados...");
}

void loop() {
  EthernetClient client = server.available(); // Espera o cliente

  if (client) {
    Serial.println("\n--- Novo Cliente Conectado ---");
    
    // Aquisição do cliente recebida
    long requestTimeout = millis() + 500; // Meio segundo de timeout
    while(client.available() == 0) {
      if (millis() > requestTimeout) {
        Serial.println("Timeout na requisicao do cliente.");
        client.stop();
        return;
      }
    }

    // Limpa qualquer dado da requisição do cliente para liberar o buffer de entrada
    while(client.available()) {
      client.read();
    }

    Serial.println("Enviando cabecalho HTTP...");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/octet-stream"); 
    client.print("Content-Length: ");
    client.println(TARGET_BYTES); // Informa ao cliente o tamanho total do "arquivo"
    client.println("Content-Disposition: attachment; filename=\"dummy_20MB.bin\"");
    client.println("Connection: close"); // A conexao sera fechada apos o envio
    client.println(); 

    Serial.println("Iniciando envio do fluxo de dados...");
    unsigned long bytesSent = 0;
    long startTime = millis();

    while (bytesSent < TARGET_BYTES) {
      if (!client.connected()) {
        Serial.println("ERRO: Cliente desconectou antes do fim do envio.");
        break;
      }
      
      // Verifica o espaço livre no buffer w5500
      int availableBuffer = client.availableForWrite();

      if (availableBuffer > 0) {
        // Verifica quantos bytes da para enviar
        int bytesToWrite = CHUNK_SIZE;

        if (bytesToWrite > availableBuffer) {
          bytesToWrite = availableBuffer;
        }
        
        unsigned long remainingBytes = TARGET_BYTES - bytesSent;
        if (bytesToWrite > remainingBytes) {
          bytesToWrite = remainingBytes;
        }

        // Envia os dados
        size_t written = client.write(dataChunk, bytesToWrite);

        if (written > 0) {
          bytesSent += written;
        }
      }
      // Se 'availableBuffer' for 0, o loop simplesmente continua e tentara novamente 
      // na proxima iteracao, sem precisar de um 'delay()', o que e mais eficiente.
      // =================================================================
    }
    
    long endTime = millis();
    // Evita divisão por zero se o tempo for muito curto
    if (endTime == startTime) {
      endTime++; 
    }
    float duration = (endTime - startTime) / 1000.0;
    float speedMbps = (bytesSent * 8.0) / (duration * 1000.0 * 1000.0);

    Serial.println("--- Envio Concluido ---");
    Serial.print("Total de bytes enviados: ");
    Serial.println(bytesSent);
    Serial.print("Tempo total: ");
    Serial.print(duration, 2);
    Serial.println(" segundos");
    Serial.print("Velocidade media: ");
    Serial.print(speedMbps, 2);
    Serial.println(" Mbps");

    delay(10);
    client.stop();
    Serial.println("Cliente desconectado.");
  }
}

