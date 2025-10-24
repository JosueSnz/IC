IPAddress ip(192, 168, 1, 132);     
IPAddress subnet(255, 255, 255, 0); 
IPAddress eth_DNS(192, 168, 1, 1);  
IPAddress gateway(192, 168, 1, 1);  

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
#define RESET_P 10
#define CS_PIN 7 


const char* mqttServer = "broker.hivemq.com"; 
const int mqttPort = 1883; 
const char* mqttUser = "";    
const char* mqttPassword = "";
const char* clientId = "micro"; 
const char* mqttTopic = "UNIFEI/EE/LABTEL/DADOS_JSON";

// micro
// 1234a34&!M
