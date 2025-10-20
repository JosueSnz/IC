
// IPAddress ip(192, 168, 1, 103);    // *** CHANGE THIS to something relevant for YOUR LAN. ***
// IPAddress subnet(255, 255, 255, 0);   // Subnet mask.
// //IPAddress eth_DNS(192, 168, 1, 1);    // *** CHANGE THIS to match YOUR DNS server.           ***
// IPAddress gateway(192, 168, 1, 1);   // *** CHANGE THIS to match YOUR Gateway (router).     ***

// IPAddress ip(169, 254, 195, 83);    // *** CHANGE THIS to something relevant for YOUR LAN. ***
// IPAddress subnet(255, 255, 0, 0);   // Subnet mask.
// //IPAddress eth_DNS(192, 168, 1, 1);    // *** CHANGE THIS to match YOUR DNS server.           ***
// IPAddress gateway(192, 168, 1, 1);   // *** CHANGE THIS to match YOUR Gateway (router).     ***

IPAddress ip(192, 168, 0, 15);    // *** CHANGE THIS to something relevant for YOUR LAN. ***
IPAddress subnet(255, 255, 255, 0);   // Subnet mask.
//IPAddress eth_DNS(192, 168, 1, 1);    // *** CHANGE THIS to match YOUR DNS server.           ***
IPAddress gateway(192, 168, 0, 1);   // *** CHANGE THIS to match YOUR Gateway (router).     ***


byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
#define RESET_P 4       // Tie the Wiz820io/W5500 reset pin to ESP32 GPIO26 pin.
#define CS_PIN 5 // Chip Select para o W5500



