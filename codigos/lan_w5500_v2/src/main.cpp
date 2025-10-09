#include <Arduino.h>
#include <SPI.h>
#include <EthernetESP32.h>
#include "local_config.h" // Inclui nosso arquivo de configuração de rede

// --- Constantes para o teste de carga ---
// Define o tamanho exato do pacote a ser enviado: 20 Megabytes
const unsigned long TARGET_BYTES = 20UL * 1024UL * 1024UL; 

// Tamanho do buffer para enviar os dados em pedaços. 
// Um valor maior pode ser mais rápido, mas consome mais RAM. 1460 é um bom valor para Ethernet.
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

  // Preenche nosso buffer com dados para serem enviados
  // Pode ser qualquer coisa, aqui estou usando a letra 'J' de Josué :)
  memset(dataChunk, 'J', CHUNK_SIZE);

  // --- Configuração da Ethernet ---
  // A inicialização do pino CS é crucial para o ESP32
  Ethernet.init(5); // Pino 5 é comum para o ESP32 com W5500. Ajuste se necessário.

  Serial.println("Iniciando conexao Ethernet...");
  Ethernet.begin(mac, ip);

  // Verificação de hardware e conexão
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Shield Ethernet nao encontrado. Verifique as conexoes SPI e o pino CS.");
    while (true) delay(1); // Trava aqui
  }
  
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Cabo de rede desconectado.");
  }

  // Aguarda um instante para o IP ser estabelecido
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
  EthernetClient client = server.available(); // Ouve por novos clientes

  if (client) {
    Serial.println("\n--- Novo Cliente Conectado ---");
    
    // Um truque para garantir que o cabeçalho da requisição do cliente foi recebido
    // antes de começarmos a enviar nossa resposta.
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

    // --- Envio da Resposta HTTP ---
    Serial.println("Enviando cabecalho HTTP...");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/octet-stream"); // Tipo de dado para download de arquivo binario
    client.print("Content-Length: ");
    client.println(TARGET_BYTES); // Informa ao cliente o tamanho total do "arquivo"
    client.println("Content-Disposition: attachment; filename=\"dummy_20MB.bin\""); // Sugere um nome de arquivo
    client.println("Connection: close"); // A conexao sera fechada apos o envio
    client.println(); // Linha em branco, fim dos cabecalhos

    // --- Loop de Envio dos Dados com Controle de Fluxo ---
    Serial.println("Iniciando envio do fluxo de dados...");
    unsigned long bytesSent = 0;
    long startTime = millis();

    while (bytesSent < TARGET_BYTES) {
      if (!client.connected()) {
        Serial.println("ERRO: Cliente desconectou antes do fim do envio.");
        break;
      }

      // ======================= MUDANCA PRINCIPAL =======================
      // Verifica quanto espaco esta livre no buffer de transmissao do W5500.
      // Esta e a chave para evitar o congestionamento.
      int availableBuffer = client.availableForWrite();

      if (availableBuffer > 0) {
        // Calcula quantos bytes podemos enviar agora.
        // Sera o menor valor entre: o CHUNK_SIZE, o espaco disponivel no buffer, 
        // ou o total de bytes que ainda faltam para enviar.
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

    // Dá um tempo para o navegador processar e fecha a conexão
    delay(10);
    client.stop();
    Serial.println("Cliente desconectado.");
  }
}

