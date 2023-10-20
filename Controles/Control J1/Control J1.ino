#include <esp_now.h>
#include <WiFi.h>

//Boton Strummer
const int botonPin1 = 15; // Variable para rastrear el estado del botón
//Boton Strummer2
const int botonPin2 = 2;
//Boton Verde
const int botonPinV = 0;
//Boton Rojo
const int botonPinR = 4;
//Boton Amarillo
const int botonPinA = 16;
//Boton Azul
const int botonPinAz = 17;
//Boton Naranja
const int botonPinN = 5;

// MAC Address del receptor
uint8_t broadcastAddress[] = {0xC8, 0xF0, 0x9E, 0x4B, 0xEB, 0xB4};

// Estructura para enviar los datos
typedef struct struct_message {
    int id; / único pra identificar al que envía
    int valV;
    int valR;
    int valA;
    int valAz;
    int valN;
} struct_message;

// Crear la estructura y llamarla myData
struct_message myData;

// Crear peer interface
esp_now_peer_info_t peerInfo;

// callback cuando se envíen los datos
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Configuración de los botones
  pinMode(botonPin1, INPUT_PULLUP);
  pinMode(botonPin2, INPUT_PULLUP);
  pinMode(botonPinV, INPUT_PULLUP);
  pinMode(botonPinR, INPUT_PULLUP);
  pinMode(botonPinA, INPUT_PULLUP);
  pinMode(botonPinAz, INPUT_PULLUP);
  pinMode(botonPinN, INPUT_PULLUP);

  // Inicializar Serial Monitor
  Serial.begin(115200);

  // Establecer al dispositivo como Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Una vez que ESP Now se haya iniciado correctamente, 
  //nos registraremos para enviar CB para obtener el estado de los datos
  esp_now_register_send_cb(OnDataSent);
  
  // Registrar peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Añadir peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  //Variables para verificar el estado de cada boton
  int estadoBoton1 = digitalRead(botonPin1);
  int estadoBoton2 = digitalRead(botonPin2);
  int estadoBotonV = digitalRead(botonPinV);
  int estadoBotonR = digitalRead(botonPinR);
  int estadoBotonA = digitalRead(botonPinA);
  int estadoBotonAz = digitalRead(botonPinAz);
  int estadoBotonN = digitalRead(botonPinN);
  
  // Establecer valor de iD
  myData.id = 1;

  // Verificar el estado de los botones del strummer 
  if (estadoBoton1 == LOW || estadoBoton2 == LOW) {
      Serial.println("Botón strummer presionado: 1");
      if (estadoBotonV == HIGH) {
        Serial.println("Botón V no presionado: 0");
        myData.valV = 0;
      } else{
        Serial.println("Botón V presionado: 1");
        myData.valV = 1;
      }
      if (estadoBotonR == HIGH) {
        Serial.println("Botón R no presionado: 0");
        myData.valR = 0;
      } else{
        Serial.println("Botón R presionado: 1");
        myData.valR = 1;
      }
      if (estadoBotonA == HIGH) {
        Serial.println("Botón A no presionado: 0");
        myData.valA = 0;
      } else{
        Serial.println("Botón A presionado: 1");
        myData.valA = 1;
      }
      if (estadoBotonAz == HIGH) {
        Serial.println("Botón Az no presionado: 0");
        myData.valAz = 0;
      } else{
        Serial.println("Botón Az presionado: 1");
        myData.valAz = 1;
      }
      if (estadoBotonN == HIGH) {
        Serial.println("Botón N no presionado: 0");
        myData.valN = 0;
      } else{
        Serial.println("Botón N presionado: 1");
        myData.valN = 1;
      }


      // Enviar los datos mediante ESP-NOW
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      
      // Indicar el estado de los datos enviados
      if (result == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      }
  }else{

  }

  delay(10);
}