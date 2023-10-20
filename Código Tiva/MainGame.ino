//***************************************************************************************************************************************
/* 
 Código para proyecto Clone Clon Hero de Digital 2
 Jorge Muñiz y Angela Quezada, Octubre de 2023
 
 Utilizando librería para el uso de la pantalla ILI9341 en modo 8 bits
 Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 Con ayuda de: José Guerra
 IE3027: Electrónica Digital 2 - 2019
*/
//***************************************************************************************************************************************
//Incluir librerías necesarias
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include <SPI.h>
#include <SD.h>
#include "Energia.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "pitches.h"
#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"
//Pines para LCD
#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
const int botones[] = {10, 39, 37, 36, 35, 34, 33, 32, 31, 30}; //Entrada de controles con pullup físico
int cuenta = 0; //Contador para generar delay antes de las notas musicales
int scoreP1 = 0; //Score de jugadores
int scoreP2 = 0; //Score de jugadores
int color = 0x03d6; //color de fondo 
//Dimensiones de pantalla
const int largo = 320; 
const int alto = 240;
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7}; //Pines LCD
int notePos[] = {4 + 41, 41 + 35, 41 + 66, 41 + 97, 41 + 128}; //Posición de botones en la pantalla
File mainLogo; //archivos para almacenar info de sd
File sSelect;
int speed = 5; //Número de pixeles que se mueve cada nota
struct vector { //Estructura para generar las notas
  int x;
  int y;
  vector() : x(0), y(0) { }
};
//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void mapeoSD(File& myFile);
int asciitohex (int a);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);
void drawFrets1(void);
void drawFrets2(void);
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);
void startPlay(void);

//Sprites para distintas pantallas
extern uint8_t bellotaGanadora[];
extern uint8_t sansGanador[];
extern uint8_t bellotaStand70[];
extern uint8_t sansStand70[];

//***************************************************************************************************************************************
// Canciones y duraciones
//***************************************************************************************************************************************
int GravityFallsNotes[] = {
  //intro
  NOTE_F5, NOTE_D5, NOTE_A4, NOTE_D5,
  NOTE_F5, NOTE_D5, NOTE_A4, NOTE_D5,
  NOTE_F5, NOTE_C5, NOTE_A4, NOTE_C5,
  NOTE_F5, NOTE_C5, NOTE_A4, NOTE_C5,
  NOTE_E5, NOTE_CS5, NOTE_A4, NOTE_CS5,
  NOTE_E5, NOTE_CS5, NOTE_A4, NOTE_CS5,
  NOTE_E5, NOTE_CS5, NOTE_A4, NOTE_CS5,
  NOTE_E5, NOTE_CS5, NOTE_E5, NOTE_CS5,
  NOTE_CS5,
  //body
  NOTE_D5, NOTE_E5, NOTE_F5, NOTE_A5,
  NOTE_G5, NOTE_A5, NOTE_C5, NOTE_D5,
  NOTE_E5, NOTE_F5, NOTE_E5, NOTE_G5,
  NOTE_A5, NOTE_G5, NOTE_F5, silence,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_A5,
  NOTE_A5, NOTE_G5, NOTE_F5, silence,
  NOTE_A5, NOTE_A5, NOTE_A5, NOTE_G5,
  NOTE_A5, NOTE_G5, NOTE_F5, silence,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_A5,
  NOTE_A5, NOTE_G5, NOTE_F5, silence,
  NOTE_A5, NOTE_A5, NOTE_A5, silence,
  NOTE_CS6, NOTE_CS6, NOTE_CS6, silence,
  silence, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_A5, NOTE_A5, NOTE_G5, NOTE_F5,
  silence, NOTE_AS5, NOTE_AS5, NOTE_AS5,
  NOTE_G5, silence, NOTE_C6, silence,
  NOTE_A5, silence, NOTE_CS6, silence,
  NOTE_F6, NOTE_D6, NOTE_A5, NOTE_D6,
  NOTE_A5, NOTE_CS6, NOTE_E6, NOTE_CS6,
  NOTE_D6, silence, NOTE_D8
};



const int GravityFallsDurations[] =
{
  125, 125, 125, 125,
  125, 125, 125, 125,
  125, 125, 125, 125,
  125, 125, 125, 125,
  125, 125, 125, 125,
  125, 125, 125, 125,
  125, 125, 125, 125,
  125, 125, 63, 125,
  63, 1, 1125, 375,
  1500, 563, 563, 375,
  1500, 1125, 375, 750,
  750, 750, 750, 750,
  750, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 375, 375,
  375, 375, 180, 180,
  180, 180, 180, 180,
  180, 180, 180, 563,
  375
};

int startNotes[] {
  NOTE_G4, silence, NOTE_G5, silence, NOTE_D5, silence, NOTE_C5, silence, NOTE_G5, silence, NOTE_G4, silence, NOTE_G4, silence, NOTE_C5, silence, NOTE_G5, silence, NOTE_A5, silence, NOTE_G5, silence, NOTE_D5, silence, NOTE_C5, silence
};

const int startNotesDurations[] {
  176, 528, 176, 352, 176, 1410, 176, 528, 176, 352, 176, 1410, 176, 528, 176, 352, 176, 528, 176, 705, 176, 528, 176, 352, 176, 1410
};

int AntiHero[] {
  NOTE_GS4, NOTE_B4, silence, NOTE_B4,
  silence, NOTE_B4, silence, NOTE_B4,
  NOTE_A4, silence, NOTE_A4, NOTE_GS4,
  silence, NOTE_GS4, NOTE_FS4, silence,
  NOTE_FS4, silence, NOTE_FS4, silence,
  NOTE_FS4, silence, NOTE_FS4, NOTE_E4,
  NOTE_B3, silence, NOTE_GS4, NOTE_E4,
  NOTE_B3, NOTE_FS4, silence, NOTE_FS4,
  silence, NOTE_FS4, silence, NOTE_FS4,
  NOTE_E4, NOTE_B4, silence, NOTE_B4,
  silence, NOTE_B4, NOTE_CS5, NOTE_B4,
  NOTE_A4, NOTE_GS4, silence, NOTE_B4,
  silence, NOTE_B4, silence, NOTE_B4,
  silence, NOTE_B4, NOTE_A4, silence,
  NOTE_GS4, silence, NOTE_GS4, NOTE_FS4,
  silence, NOTE_FS4, silence, NOTE_FS4,
  silence, NOTE_FS4, silence, NOTE_FS4,
  silence, NOTE_FS4, silence, NOTE_FS4,
  silence, NOTE_FS4, NOTE_E4, NOTE_B3,
  NOTE_E4, silence, NOTE_FS4, silence,
  NOTE_FS4, silence, NOTE_FS4, silence,
  NOTE_FS4, silence, NOTE_FS4, silence,
  NOTE_E4, silence, NOTE_B4, silence,
  NOTE_B4, NOTE_CS5, NOTE_B4, silence,
  NOTE_E5, NOTE_GS5, silence, NOTE_GS5,
  silence, NOTE_FS5, silence, NOTE_FS5,
  silence, NOTE_FS5, silence, NOTE_FS5,
  silence, NOTE_FS5, NOTE_E5, silence,
  NOTE_E5, silence, NOTE_B4, NOTE_GS5,
  silence, NOTE_GS5, silence, NOTE_FS5,
  silence, NOTE_FS5, silence, NOTE_FS5,
  silence, NOTE_FS5, NOTE_E5, silence,
  NOTE_E5, silence, NOTE_GS5, silence,
  NOTE_GS5, silence, NOTE_GS5, silence,
  NOTE_GS5, silence, NOTE_GS5, silence,
  NOTE_GS5, silence, NOTE_GS5, NOTE_FS5,
  silence, NOTE_FS5, silence, NOTE_FS5,
  silence, NOTE_FS5, silence, NOTE_FS5,
  silence, NOTE_FS5, silence, NOTE_FS5,
  NOTE_E5, silence, NOTE_E5, silence,
  NOTE_B4, silence, NOTE_B4, silence,
  NOTE_B4, silence, NOTE_B4, NOTE_A4,
  silence, NOTE_A4, NOTE_GS4, silence,
  NOTE_FS4, silence, NOTE_FS4, silence,
  NOTE_FS4, silence, NOTE_FS4, silence,
  NOTE_FS4, silence, NOTE_FS4, silence,
  NOTE_FS4, NOTE_E4, NOTE_A5, NOTE_GS5,
  NOTE_FS5, NOTE_E5, NOTE_DS5, NOTE_E5,
  NOTE_GS5, NOTE_FS5, NOTE_E5, silence,
  NOTE_GS5, silence, NOTE_GS5, NOTE_B5,
  silence, NOTE_B5, NOTE_FS5, silence,
  NOTE_FS5, silence, NOTE_FS5, silence,
  NOTE_FS5, NOTE_DS5, silence, NOTE_DS5,
  silence, NOTE_CS5, NOTE_DS5, NOTE_CS5,
  NOTE_DS5, NOTE_E5, silence, NOTE_GS5,
  silence, NOTE_GS5, NOTE_B5, silence,
  NOTE_B5, NOTE_FS5, silence, NOTE_FS5,
  silence, NOTE_FS5, silence, NOTE_FS5
};

const int AntiHeroDuration[] {
  309, 154, 154, 154,
  154, 154, 154, 309,
  154, 154, 309, 154,
  154, 309, 154, 154,
  154, 154, 154, 154,
  154, 154, 309, 309,
  154, 154, 309, 309,
  309, 154, 154, 154,
  154, 154, 154, 309,
  309, 154, 154, 154,
  154, 309, 309, 463,
  154, 618, 309, 154,
  154, 154, 154, 154,
  154, 309, 154, 154,
  154, 154, 309, 154,
  154, 154, 154, 154,
  154, 154, 154, 154,
  154, 154, 154, 154,
  154, 309, 309, 309,
  154, 154, 154, 154,
  154, 154, 154, 154,
  154, 154, 154, 154,
  154, 154, 154, 154,
  309, 309, 616, 309,
  309, 462, 770, 462,
  770, 154, 154, 77,
  77,  231, 77, 154,
  231, 77, 309, 616,
  309, 462, 770, 462,
  770, 154, 154, 77,
  77,  231, 77, 154,
  231, 77, 309, 1232,
  154, 154, 154, 154,
  77,  77,  309, 154,
  154, 154, 154, 154,
  309, 154, 154, 154,
  154, 154, 77,  77,
  309, 154, 77, 77,
  462, 77, 77, 231,
  77, 154, 154, 154,
  154, 154, 77, 77,
  462, 77, 77, 462,
  154, 154, 154, 154,
  154, 154, 154, 154,
  77,  77,  309, 154,
  77,  77,  462, 309,
  309, 309, 309, 616,
  309, 309, 309, 924,
  1540, 309, 154, 154,
  309, 77, 77, 462,
  154, 154, 154, 154,
  77,  77,  309, 309,
  154, 154, 154, 154,
  462, 154, 309, 462,
  309, 154, 154, 309,
  77,  77,  462, 154,
  154, 154, 154, 77,
  77,  1386

};

unsigned long previousMillis; //Variable para almacenr tiempo de corrida del programa (pararelismo en generación de notas)
int indice = 0; //Posición de la canción (número de nota)
int menu = 0; //Modo de operación
//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  //Iniciar SD
  SPI.setModule(0);
  Serial.print("Initializing SD card...");
  if (!SD.begin(32)) {
    Serial.println("initialization failed!");
    return;
  }
  //Iniciar botones como input simple, el pulldown es físico
  for (int i = 0; i < sizeof(botones) / sizeof(int); i++) {
    pinMode(botones[i], INPUT);
  }
  Serial.println("initialization done.");
  //Iniciar pantalla
  LCD_Init();
  LCD_Clear(0);
}

//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  switch (menu) {
    case 0: //Menú de inicio
      previousMillis = 0; //Preparar tiempo de corrida para música de inicio
      mainLogo = SD.open("logo.txt"); //Abrir imagen de inicio desde SD
      mapeoSD(mainLogo); //Cargar imagen de inicio a pantalla
      previousMillis = millis();//Registrar el tiempo de corrida del programa
      indice = 0; //Iniciar canción en inicio de array de notas
      while (1) {
        unsigned long currentMillis = millis(); //Registrar tiempo actual

        //Revisar botones para cambiar de menú hacia selección de dificultad
        if (digitalRead(botones[4]) == 1) {
          menu = 1; //Cambiar valor de switch
          noTone(40);  //Apagar música de inicio

          //Generar cuadrados para indicar selección
          FillRect(260, 56, 8, 8, 0x8c3f);
          FillRect(260, 176, 8, 8, 0x8c3f); 
          delay(250);
          FillRect(260, 56, 8, 8, 0x101f);
          FillRect(260, 176, 8, 8, 0x101f);
          delay(500); 

          break;//Salir d While
        } else if (digitalRead(botones[9]) == 1) {
          menu = 1;
          noTone(40);
          FillRect(260, 56, 8, 8, 0x8c3f);
          FillRect(260, 176, 8, 8, 0x8c3f);
          delay(250);
          FillRect(260, 56, 8, 8, 0x101f);
          FillRect(260, 176, 8, 8, 0x101f);
          delay(500);
          break;

        }

        if ( currentMillis - previousMillis >= startNotesDurations[indice])//Utilizar tiempo pasado y tiempo actual de programa para generar pararelismo artificial utilizando duración de canciones
        {

          previousMillis = currentMillis; //Almacenar milis actuales para siguiente iteración


          if (indice % 2 == 0) { //En notas pares, tocar la nota (las intermedias son silencios)
            tone(40, startNotes[indice]);
            //Generar cuadrados sincronizados con la música
            FillRect(260, 56, 8, 8, 0x101f); //Apagado
            FillRect(260, 176, 8, 8, 0x8c3f); //Encendido
          } else {
            noTone(40); //Silencio en impares
            //Generar cuadrados sincronizados con la música
            FillRect(260, 56, 8, 8, 0x8c3f);//Encendido
            FillRect(260, 176, 8, 8, 0x101f); //Apagado
          }





          indice++; //Aumentar indice para cambiar de nota en siguiente iteración
          if ( indice >= sizeof(startNotesDurations) / sizeof(int))
          {
            indice = 0; //Reiniciar canción si se acabaron las notas

          }
        }
      }

      break;
      
    case 1://Selección de dificultad
      mainLogo = SD.open("nivel.txt"); //Abrir imagen de dificultad
      mapeoSD(mainLogo); //Cargar imagen en pantalla
      previousMillis = millis(); //Registrar tiempo de corrida de programa
      while (1) {
        unsigned long currentMillis = millis(); //Tiempo de corrida actual
        //Revisar si se slecciona dificultad y salir de ciclo while
        if (digitalRead(botones[0]) == 1) {
          speed = 1;
          break;
        } else if (digitalRead(botones[5]) == 1) {
          speed = 1;
          break;
        } else if (digitalRead(botones[2]) == 1) {
          speed = 3;
          break;
        } else if (digitalRead(botones[7]) == 1) {
          speed = 3;
          break;
        } else if (digitalRead(botones[4]) == 1) {
          speed = 5;
          break;
        } else if (digitalRead(botones[9]) == 1) {
          speed = 5;
          break;
        }

//Lógica de canción y canción igual a estado anterior
        if ( currentMillis - previousMillis >= startNotesDurations[indice])
        {

          previousMillis = currentMillis;


          if (indice % 2 == 0) {
            tone(40, startNotes[indice]);

          } else {
            noTone(40);

          }





          indice++;
          if ( indice >= sizeof(startNotesDurations) / sizeof(int))
          {
            indice = 0;

            // break;
          }
        }
      }
      noTone(40);
      menu = 2;
      break;
    case 2: //Selección de canción
      {
        sSelect = SD.open("select.txt"); //Abrir imagen
        mapeoSD(sSelect); //Cargar imagen en pantalla
        previousMillis = millis();//Registrar tiempo de programa
        int anim = 0; //Variable para cambiar sprites en cada iteración de ciclo
        while (1) {
          unsigned long currentMillis = millis(); //Registrar tiempo actual

          //Revisar selección de canción
          if (digitalRead(botones[0]) == 1) {
            menu = 3;
            break;
          } else if (digitalRead(botones[5]) == 1) {
            menu = 3;
            break;
          } else if (digitalRead(botones[1]) == 1) {
            menu = 4;
            break;
          } else if (digitalRead(botones[6]) == 1) {
            menu = 4;
            break;
          } 
          anim ++; //Aumentar número de sprite
          LCD_Sprite(241, 131, 70, 70, sansStand70, 2, (anim / 12) % 2, 0, 0); //Generar sprite jugador 1
          LCD_Sprite(241, 111 - 70, 70, 70, bellotaStand70, 4, (anim / 12) % 4, 0, 0); //Generar sprite jugador 2
          if (anim >= 200) {//Mantener contador de animación en menor de 200
            anim = 0;
          }

          if ( currentMillis - previousMillis >= startNotesDurations[indice])//Lógica de canción y canción y igual que estado anterior
          {

            previousMillis = currentMillis;


            if (indice % 2 == 0) {
              tone(40, startNotes[indice]);

            } else {
              noTone(40);

            }





            indice++;
            if ( indice >= sizeof(startNotesDurations) / sizeof(int))
            {
              indice = 0;

              // break;
            }
          }
        }


      }
      noTone(40);
      indice = 0;
      break;
    case 3:
      {
        startPlay(); //Inicializar pantalla para juego
        bool augmentScore = true; //bool para aumentar score de jugador 1 solamnte 1 vez por pulso de botón a tiempo
        bool augmentScore2 = true; //bool para aumentar score de jugador 2 solamnte 1 vez por pulso de botón a tiempo
        while (1) {
          unsigned long currentMillis = millis();
          cuenta ++;

          if ( currentMillis - previousMillis >= GravityFallsDurations[indice] && cuenta > 17) //Misma lógica de pararelismo, pero con canción diferente 
          {

            previousMillis = currentMillis;


            if (GravityFallsNotes[indice] != silence) {//Si la nota que toca no es un silencio, generar punto en pantalla
              vector *myVector = new vector(); //Crear nuevo punto en memoria dinámica cada vez que toque una nota nueva
              myVector->y = notePos[random(0, 5)]; //asignar cuerda aleatoria
              do { //Aumentar la posición del punto hasta que se encuentre en la parte inferior de la pantalla
                myVector->x += speed; //Aumentar posición del punto verticalmente

                LCD_Bitmap(myVector->x, myVector->y, 27, 27, celeste); //Generar bitmap en posición de vector
                FillRect(myVector->x - speed, myVector->y , speed, 27, 0); //Limpiar trazo del vector
                FillRect(myVector->x  - speed, myVector->y + 13, speed, 1, 0xffff); //Generar fragmento de cuerda detrás del vector



                for (int i = 0; i < 5; i++) { //Revisar todos los botones de Jugador 1
                  if (digitalRead(botones[i]) == 1 && notePos[4 - i] == myVector->y && myVector->x > largo - 56) {//revisar que se esté tocando el botón correcto en la cuerda correcta en la posición de los botones
                    if (augmentScore) {//Si no se ha presionado el botón
                      scoreP1++; //Aumentar score
                      augmentScore = false; //Indicar que se aumentó score

                    }
                  }
                }

                for (int i = 5; i < 10; i++) { //Revisar todos los botones de Jugador 2
                  if (digitalRead(botones[i]) == 1 && notePos[9 - i] == myVector->y && myVector->x > largo - 56) {//revisar que se esté tocando el botón correcto en la cuerda correcta en la posición de los botones
                    if (augmentScore2) {//Si no se ha presionado el botón
                      scoreP2++;//Aumentar score
                      augmentScore2 = false;//Indicar que se aumentó score

                    }
                  }
                }

              } while (myVector->x < largo - 28);//mientras el vector esté en la pantalla
              delete myVector;//Liberar memoria dinámica al terminar el uso de vector
            }
            noTone(40); //Apagar cualquier nota que estuviera sonando previamente
            tone(40, GravityFallsNotes[indice]); //Tocar nota actual
            augmentScore = true; //Permitir aumento de score para siguiente iteración
            augmentScore2 = true; //Permitir aumento de score para siguiente iteración
            LCD_Print("Player 1: " + String(scoreP1), 55, 14, 2, 0xffff, color); //Mostrar Score
            LCD_Print("Player 2: " + String(scoreP2), 55, alto - 28, 2, 0xffff, color); //Mostrar Score
            indice++; //Aumentar indicador de nota
            if ( indice >= sizeof(GravityFallsDurations) / sizeof(int)) //Revisar si terminó la canción
            {
              if (scoreP1 > scoreP2) {//Revisar si el jugador 1 ganó
                menu = 7;//Pantalla de victoria 1
              } else {//Si j2 ganó
                menu = 8;//pantalla de victoria 2
              }
              indice = 0;//Resetear canción para menú
              break;//salir de ciclo
            }
          }

          //dibujar sprites de jugadores a costados de la pantalla
          LCD_Sprite(largo - 32, 7, 30, 30, bellotaStand30, 4, (cuenta / 8) % 4, 0, 0);
          LCD_Sprite(largo - 32, alto - 37, 30, 30, sansStand30, 2, (cuenta / 8) % 2, 0, 0);
          //Dibujar bordes
          V_line(319, 0, alto , 0);
          V_line(318, 0, alto , 0);
          V_line(317, 0, alto , 0);
          //Dibujar botones
          drawFrets1();
        }
      }
      break;
    case 4:
      {
        //Misma lógica que estado anterior, pero con canción distinta
        startPlay();
        bool augmentScore = true;
        bool augmentScore2 = true;
        while (1) {
          unsigned long currentMillis = millis();
          cuenta ++;

          if ( currentMillis - previousMillis >= AntiHeroDuration[indice] && cuenta > 17)
          {

            previousMillis = currentMillis;


            if (AntiHero[indice] != silence) {
              vector *myVector = new vector();
              myVector->y = notePos[random(0, 5)];
              do {
                myVector->x += speed;

                LCD_Bitmap(myVector->x, myVector->y, 27, 27, celeste);
                FillRect(myVector->x - speed, myVector->y , speed, 27, 0);
                FillRect(myVector->x  - speed, myVector->y + 13, speed, 1, 0xffff);



                for (int i = 0; i < 5; i++) {
                  if (digitalRead(botones[i]) == 1 && notePos[4 - i] == myVector->y && myVector->x > largo - 56) {
                    if (augmentScore) {
                      scoreP1++;
                      augmentScore = false;

                    }
                  }
                }

                for (int i = 5; i < 10; i++) {
                  if (digitalRead(botones[i]) == 1 && notePos[9 - i] == myVector->y && myVector->x > largo - 56) {
                    if (augmentScore2) {
                      scoreP2++;
                      augmentScore2 = false;

                    }
                  }
                }

              } while (myVector->x < largo - 28);
              delete myVector;
            }
            noTone(40);
            tone(40, AntiHero[indice]);
            augmentScore = true;
            augmentScore2 = true;
            LCD_Print("Player 1: " + String(scoreP1), 55, 14, 2, 0xffff, color);
            LCD_Print("Player 2: " + String(scoreP2), 55, alto - 28, 2, 0xffff, color);
            indice++;
            if ( indice >= sizeof(AntiHeroDuration) / sizeof(int))
            {
              noTone(40);
              if (scoreP1 > scoreP2) {
                menu = 7;
              } else {
                menu = 8;
              }
              indice = 0;
              break;
            }
          }


          LCD_Sprite(largo - 32, 7, 30, 30, bellotaStand30, 4, (cuenta / 8) % 4, 0, 0);
          LCD_Sprite(largo - 32, alto - 37, 30, 30, sansStand30, 2, (cuenta / 8) % 2, 0, 0);
          V_line(319, 0, alto , 0);
          V_line(318, 0, alto , 0);
          V_line(317, 0, alto , 0);

          drawFrets1();
        }
      }
      break;
    case 5:
      Serial.println("song3");//PLace holder
      menu = 0;
      break;
    case 6:
      Serial.println("song4");//Place Holder
      menu = 0;
      break;
    case 7:
      {
        sSelect = SD.open("onew.txt");//Cargar victoria j1
        mapeoSD(sSelect);//mostrar en pantalla
        int anim = 0;//Variable para animación de sprite
        while (1) {
          //Revisar botones pra volver a jugar y resetear estados
          if (digitalRead(botones[0]) == 1) {
            menu = 0;
            break;
          } else if (digitalRead(botones[5]) == 1) {
            menu = 0;
            break;
          }
          anim++;//aumntar sprite
          LCD_Sprite(167, 15, 100, 100, bellotaGanadora, 3, (anim / 12) % 3, 0, 0);//Dibujar sprite ganador
          if (anim >= 200) {//mantener animación mnor a 200
            anim = 0;
          }
        }
      }
      break;
    case 8:
      {
        sSelect = SD.open("twow.txt");//Cargar victoria j1
        mapeoSD(sSelect);//mostrar en pantalla
        int anim = 0;//Variable para animación de sprite
        while (1) {
          //Revisar botones pra volver a jugar y resetear estados
          if (digitalRead(botones[0]) == 1) {
            menu = 0;
            break;
          } else if (digitalRead(botones[5]) == 1) {
            menu = 0;
            break;
          }
          anim++;//aumntar sprite
          LCD_Sprite(167, 124, 100, 100, sansGanador, 2, (anim / 12) % 2, 0, 0);//Dibujar sprite ganador
          if (anim >= 200) {//mantener animación mnor a 200
            anim = 0;
          }
        }

      }
      break;
  }



}
//***************************************************************************************************************************************
// Función para convertir de ascii a hex (SD a pantalla)
//***************************************************************************************************************************************
int asciitohex(int a) {
  switch (a) {
    case 48: //Si se lee un '0'
      return 0x00; //Devolver un 0
    case 49: //Si se lee un '1'
      return 0x01; //Devolver un 1
    case 50: //Si se lee un '2'
      return 0x02; //Devolver un 2
    case 51: //Si se lee un '3'
      return 0x03; //Devolver un 3
    case 52: //Si se lee un '4'
      return 0x04; //Devolver un 4
    case 53: //Si se lee un '5'
      return 0x05; //Devolver un 5
    case 54: //Si se lee un '6'
      return 0x06; //Devolver un 6
    case 55: //Si se lee un '7'
      return 0x07; //Devolver un 7
    case 56: //Si se lee un '8'
      return 0x08; //Devolver un 8
    case 57: //Si se lee un '9'
      return 0x09; //Devolver un 9
    case 97: //Si se lee un 'a'
      return 0x0A; //Devolver un 10
    case 98: //Si se lee un 'b'
      return 0x0B; //Devolver un 11
    case 99: //Si se lee un 'c'
      return 0x0C; //Devolver un 12
    case 100: //Si se lee un 'd'
      return 0x0D; //Devolver un 13
    case 101: //Si se lee un 'e'
      return 0x0E; //Devolver un 14
    case 102: //Si se lee un 'f'
      return 0x0F; //Devolver un 15
  }
}
//***************************************************************************************************************************************
// Función para iniciar partida (pintar figuras permanentes)
//***************************************************************************************************************************************
void startPlay(void) {
  LCD_Clear(0);//limpiar pantalla
  //Cuerdas
  for (int i = 17 + 40; i < 142 + 40; i = i + 31) {
    H_line( 0, i, largo, 0xffff);

  }
  //Bordes
  FillRect(0, 0, largo, 44, color);
  FillRect(0, alto - 44, largo, 44, color);
  H_line( 0, 44, largo, 0x00df);
  H_line( 0, 43, largo, 0x00df);
  H_line( 0, alto - 44, largo, 0x00df);
  H_line( 0, alto - 43, largo, 0x00df);


  drawFrets1();//Dibujar botones
  indice = 0; //Iniciar canción en 0
  previousMillis = 0; //iniciar tiempo
  previousMillis = millis();
  scoreP1=0;
  scoreP2=0;
}

//***************************************************************************************************************************************
// Función para mostrar archivos de SD en pantalla
//***************************************************************************************************************************************

void mapeoSD(File& myFile) {
  int hex1 = 0; //Variable para almacenar lo leído de la SD
  int val1 = 0; //Decena hexadecimal
  int val2 = 0; //Unidad hexadecimal
  int mapear = 0; //Iniciar con la posición del arreglo en 0
  int vertical = 0; //Iniciar con la coordenada en y en 0
  unsigned char maps[640]; //Definir un arreglo de 640 espacios

  if (myFile) {
    while (myFile.available()) {
      mapear = 0;
      while (mapear < 640) {
        hex1 = myFile.read(); //Leer archivo y guardar en variable para comparar
        if (hex1 == 120) { //Si se lee "x" guardar unidad y decena (0x00)
          val1 = myFile.read(); //Almacenar decena
          val2 = myFile.read(); //Almacenar unidad
          val1 = asciitohex(val1); //ASCII a hexadecimal
          val2 = asciitohex(val2); //ASCII a hexadecimal
          maps[mapear] = val1 * 16 + val2; //Guardar el valor de mapeo en el arreglo
          mapear++; //Aumentar la posición del arreglo

        }
      }

      LCD_Bitmap(0, vertical, 320, 1, maps); //Mostrar una fila de contenido de la imagen
      vertical++; //Aumentar la línea vertical

      if (vertical >= 239) {
        myFile.close();
        break;
      }
    }

    myFile.close(); //Cerrar el archivo si ya hay contenido por leer
  }
  else {
    myFile.close(); //Si no se encontró el arhivo cerrar para evitar algún error

  }
}


//***************************************************************************************************************************************
// Función para dibujar botones al fondo de la pantalla
//***************************************************************************************************************************************

void drawFrets1(void) {
  LCD_Bitmap(largo - 30, 4 + 41, 27, 27, verdeVacio);
  LCD_Bitmap(largo - 30, 35 + 41, 27, 27, rojoVacio);
  LCD_Bitmap(largo - 30, 66 + 41, 27, 27, amarilloVacio);
  LCD_Bitmap(largo - 30, 97 + 41, 27, 27, azulVacio);
  LCD_Bitmap(largo - 30, 128 + 41, 27, 27, naranjaVacio);
}

//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER)
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
    }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y + h, w, c);
  V_line(x  , y  , h, c);
  V_line(x + w, y  , h, c);
}


void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + w;
  y2 = y + h;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = w * h * 2 - 1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);

      //LCD_DATA(bitmap[k]);
      k = k - 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;

  if (fontSize == 1) {
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if (fontSize == 2) {
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }

  char charInput ;
  int cLength = text.length();
  Serial.println(cLength, DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength + 1];
  text.toCharArray(char_array, cLength + 1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1) {
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2) {
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      //LCD_DATA(bitmap[k]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  if (flip) {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width - 1 - offset) * 2;
      k = k + width * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k - 2;
      }
    }
  } else {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width + 1 + offset) * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k + 2;
      }
    }


  }
  digitalWrite(LCD_CS, HIGH);
}
