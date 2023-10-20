#include <esp_now.h>
#include <WiFi.h>

// Pines de salida
const int pinLEDV2 = 4; 
const int pinLEDR2 = 16; 
const int pinLEDA2 = 17; 
const int pinLEDAz2 = 5; 
const int pinLEDN2 = 18; 
const int pinLEDV = 12; 
const int pinLEDR = 14; 
const int pinLEDA = 27; 
const int pinLEDAz = 26; 
const int pinLEDN = 25; 

// Estructura para recibir los datos
// Es igual a la del emisor
typedef struct struct_message {
  int id;
  int valV;
  int valR;
  int valA;
  int valAz;
  int valN;
}struct_message;

// Crear la estructura y llamarla myData
struct_message myData;

// Crear una estructura para almacenar las lecturas de cada emisor
struct_message board1;
struct_message board2;

// Crear un array con todas las estructuras
struct_message boardsStruct[2] = {board1, board2};

// callback cuando se reciban los datos
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);

  // Actualizar la estructura con los nuevos datos
  boardsStruct[myData.id-1].valV = myData.valV;
  boardsStruct[myData.id-1].valR = myData.valR;
  boardsStruct[myData.id-1].valA = myData.valA;
  boardsStruct[myData.id-1].valAz = myData.valAz;
  boardsStruct[myData.id-1].valN = myData.valN;
  Serial.printf("valV value: %d \n", boardsStruct[myData.id-1].valV);
  Serial.printf("valR value: %d \n", boardsStruct[myData.id-1].valR);
  Serial.printf("valA value: %d \n", boardsStruct[myData.id-1].valA);
  Serial.printf("valAz value: %d \n", boardsStruct[myData.id-1].valAz);
  Serial.printf("valN value: %d \n", boardsStruct[myData.id-1].valN);
  Serial.println();

   // Acceder a las varaibles de cada board
  int board1V = boardsStruct[0].valV;
  int board1R = boardsStruct[0].valR;
  int board1A = boardsStruct[0].valA;
  int board1Az = boardsStruct[0].valAz;
  int board1N = boardsStruct[0].valN;
  int board2V = boardsStruct[1].valV;
  int board2R = boardsStruct[1].valR;
  int board2A = boardsStruct[1].valA;
  int board2Az = boardsStruct[1].valAz;
  int board2N = boardsStruct[1].valN;
  //Serial.println(board1V);
  
  //Verificar el valor que contiene cada variable 
  if (board1V == 1) {
    digitalWrite(pinLEDV, HIGH);  // Enciende el LED
  } else {
    digitalWrite(pinLEDV, LOW);  // Apaga el LED
  }
  if (board1R == 1) {
    digitalWrite(pinLEDR, HIGH);  // Enciende el LED
  } else {
    digitalWrite(pinLEDR, LOW);  // Apaga el LED
  }
  if (board1A == 1) {
    digitalWrite(pinLEDA, HIGH);  // Enciende el LED
  } else {
    digitalWrite(pinLEDA, LOW);  // Apaga el LED
  }
  if (board1Az == 1) {
    digitalWrite(pinLEDAz, HIGH);  // Enciende el LED
  } else {
    digitalWrite(pinLEDAz, LOW);  // Apaga el LED
  }
  if (board1N == 1) {
    digitalWrite(pinLEDN, HIGH);  // Enciende el LED
  } else {
    digitalWrite(pinLEDN, LOW);  // Apaga el LED
  }
  if (board2V == 1) {
    digitalWrite(pinLEDV2, HIGH);  // Enciende el LED
  } else {
    digitalWrite(pinLEDV2, LOW);  // Apaga el LED
  }
  if (board2R == 1) {
    digitalWrite(pinLEDR2, HIGH);  // Enciende el LED
  } else {
    digitalWrite(pinLEDR2, LOW);  // Apaga el LED
  }
  if (board2A == 1) {
    digitalWrite(pinLEDA2, HIGH);  // Enciende el LED
  } else {
    digitalWrite(pinLEDA2, LOW);  // Apaga el LED
  }
  if (board2Az == 1) {
    digitalWrite(pinLEDAz2, HIGH);  // Enciende el LED
  } else {
    digitalWrite(pinLEDAz2, LOW);  // Apaga el LED
  }
  if (board2N == 1) {
    digitalWrite(pinLEDN2, HIGH);  // Enciende el LED
  } else {
    digitalWrite(pinLEDN2, LOW);  // Apaga el LED
  }
  delay(10);

  //Resetear los pines para evitar que se queden encendidos 
  digitalWrite(pinLEDV, LOW);
  digitalWrite(pinLEDR, LOW);
  digitalWrite(pinLEDA, LOW);
  digitalWrite(pinLEDAz, LOW);
  digitalWrite(pinLEDN, LOW);
  digitalWrite(pinLEDV2, LOW);
  digitalWrite(pinLEDR2, LOW);
  digitalWrite(pinLEDA2, LOW);
  digitalWrite(pinLEDAz2, LOW);
  digitalWrite(pinLEDN2, LOW);
}
 
void setup() {
  //Configurar pines de salida
  pinMode(pinLEDV2,OUTPUT);
  pinMode(pinLEDR2,OUTPUT);
  pinMode(pinLEDA2,OUTPUT);
  pinMode(pinLEDAz2,OUTPUT);
  pinMode(pinLEDN2,OUTPUT);
  pinMode(pinLEDV,OUTPUT);
  pinMode(pinLEDR,OUTPUT);
  pinMode(pinLEDA,OUTPUT);
  pinMode(pinLEDAz,OUTPUT);
  pinMode(pinLEDN,OUTPUT);
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Una vez que ESP Now se haya iniciado correctamente, 
  //nos registraremos para enviar CB para obtener el estado de los datos
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
 

  delay(10);  
}