#include <Arduino.h>
#include <Time.h>
#include <TimeLib.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "rgb_lcd.h"
#include "TM1637Display.h"

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const int SEG7_CLK = 7;
const int SEG7_DIO = 6;
TM1637Display seg7Display(SEG7_CLK, SEG7_DIO);
uint8_t seg7_data[4] = { 0xff, 0xff, 0xff, 0xff };

long randNumber;

rgb_lcd lcd;
const int rojo[3] = {255, 0 , 0};
const int verde[3] = {0, 255 , 0};
const int azul[3] = {0, 0 , 255};
const int amarillo[3] = {255, 255 , 255};
const int rosado[3] = {255, 192 , 203};

const int estado1_SELECCIONAR_N_PARTICIPANTES = 1;
const int estado2_ELIJA_TIEMPO = 2;
const int estado3_SELECCIONANDO_ACTOR = 3;
const int estado4_ESPERAR_COMANDO = 4;
const int estado5_CUENTA_ATRAS = 5;


const int comando_OK = 1;
const int comando_CANCELAR = 2;

int estado_anterior  = 0;
int state  = estado1_SELECCIONAR_N_PARTICIPANTES;//Estado inicial
int nParticipantes  = 2;//minimo 2, maximo 5
int nDuracion  = 1;//en minutos
int participanteActual = 0;
int comandoActual = comando_OK;
time_t periodoActual = 60;
time_t tiempoInicio = 0;
time_t tiempoPasado = 0;
time_t tiempoActual = 0;
time_t tiempoAnterior = 0;
const int botonSET = 2;
const int botonNEXT = 3;

const int salidaControl = 4;

int estadoBotonSET;
int estadoBotonSET_pre;

int estadoBotonNEXT;
int estadoBotonNEXT_pre;


//VARIABLES PARA EL MODULO dE COMUNICACIONES RF
#define CE_PIN 9
#define CSN_PIN 10
byte direccionRF[5] = {'p', 'u', 'c', 'p', '0'};
RF24 radio(CE_PIN, CSN_PIN);
byte estadoActualEquipos[7];
int counterTX = 0;
inline void comunicarEstadoActual(bool participandoQ, int tiempoRestante) {

  if (counterTX == 1) {
    estadoActualEquipos[0] = 0xFF;
    estadoActualEquipos[1] = participandoQ ? 1 : 0;
    estadoActualEquipos[2] = nParticipantes;
    estadoActualEquipos[3] = participanteActual;
    int centenas = tiempoRestante / 100;
    int decenas = (tiempoRestante - centenas * 100) / 10;
    int unidades = tiempoRestante % 10;
    estadoActualEquipos[4] = centenas;
    estadoActualEquipos[5] = decenas;
    estadoActualEquipos[6] = unidades;
    //    estadoActualEquipos[0] = 0;
    //    estadoActualEquipos[1] = 1;
    //    estadoActualEquipos[2] = 2;
    //    estadoActualEquipos[3] = 3;
    //    estadoActualEquipos[4] = 4;
    //    estadoActualEquipos[5] = 5;
    //    estadoActualEquipos[6] = 6;
    bool ok = radio.write(estadoActualEquipos, sizeof(estadoActualEquipos));
    Serial.print("Dato enviado 0: ");
    Serial.println(String((int)estadoActualEquipos[0]));
    Serial.print("Dato enviado 1: ");
    Serial.println(String((int)estadoActualEquipos[1]));
    Serial.print("Dato enviado 2: ");
    Serial.println(String((int)estadoActualEquipos[2]));
    Serial.print("Dato enviado 3: ");
    Serial.println(String((int)estadoActualEquipos[3]));
    Serial.print("Dato enviado 4: ");
    Serial.println(String((int)estadoActualEquipos[4]));
    Serial.print("Dato enviado 5: ");
    Serial.println(String((int)estadoActualEquipos[5]));
    Serial.print("Dato enviado 6: ");
    Serial.println(String((int)estadoActualEquipos[6]));
  }
  counterTX++;
  counterTX %= 2;
}
///////////////////////////////////////////////


inline void digitos_limpiar() {
  seg7Display.clear();
}

inline void digitos_mostrar(int numero) {
  seg7Display.showNumberDec(numero, true);
}

inline void cambiarEstado(int estadoNuevo) {
  estado_anterior = state;
  state = estadoNuevo;
}

inline int estadoActual() {
  return state;
}

inline int estadoAnterior() {
  return estado_anterior;
}

inline void avanzarEstado() {
  cambiarEstado(estadoActual() + 1);
}

int publicoActual = 7;
inline void seleccionarActor() {
  cambiarEstado(estado3_SELECCIONANDO_ACTOR);
  participanteActual = 0;
  publicoActual = random(1, 20);
}

inline void iniciarCuenta() {
  cambiarEstado(estado5_CUENTA_ATRAS);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Actor: ");
  mostrarParticipante();
  periodoActual = nDuracion * 60;
  tiempoInicio = now();
  tiempoActual = tiempoInicio;
}

inline void mostrarParticipante() {

  if (participanteActual < nParticipantes) {
    lcd.print(String(participanteActual + 1));
  } else {
    if (participanteActual == nParticipantes) {
      lcd.print("Publico");
    } else {
      lcd.print("Publico(Azar)");
    }

  }
}

bool enPublicoQ() {
  if (participanteActual == nParticipantes) {
    return true;
  }
  if (participanteActual == nParticipantes + 1) {
    return true;
  }
  return false;
}

inline bool tiempoLibreQ() {
  return (nDuracion == 6);
}

void evaluarEstado() {
  if (
    (estadoActual() == estado1_SELECCIONAR_N_PARTICIPANTES)
  ) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("# Participantes:");
    lcd.setCursor(0, 1);
    lcd.print(String(nParticipantes));
    lcd.blink();
    comunicarEstadoActual(false, 0);

  } else if (estadoActual() == estado2_ELIJA_TIEMPO) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tiempo?(m):");
    lcd.setCursor(0, 1);
    if (tiempoLibreQ()) {
      lcd.print("Libre");
    } else {
      lcd.print(String(nDuracion));
    }
    lcd.blink();
    comunicarEstadoActual(false, 0);
    //lcd.setRGB(rosado[0], rosado[1], rosado[2]);
  } else if (estadoActual() == estado3_SELECCIONANDO_ACTOR) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sgte. Actor:");
    lcd.setCursor(0, 1);
    mostrarParticipante();
    lcd.blink();
    comunicarEstadoActual(false, 0);
  } else if (estadoActual() == estado4_ESPERAR_COMANDO) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Proceder?");
    lcd.setCursor(0, 1);
    if (comandoActual == comando_OK) {
      lcd.print("OK");
    } else {
      lcd.print("CANCELAR");
    }
    lcd.blink();
    comunicarEstadoActual(false, 0);
  } else if (estadoActual() == estado5_CUENTA_ATRAS) {

    tiempoPasado = (tiempoActual - tiempoInicio);
    if ( (periodoActual < tiempoPasado) && (!tiempoLibreQ()) && !enPublicoQ() ) {
      seleccionarActor();
      evaluarEstado();
    } else {
      lcd.setCursor(0, 1);
      if (tiempoLibreQ()) {
        lcd.print("Duracion: ");
        lcd.print(String(tiempoPasado));
        digitos_mostrar(tiempoPasado);
        comunicarEstadoActual(true, tiempoPasado);
      } else {
        if (participanteActual < nParticipantes) {
          lcd.print("Restante: ");
          int aMostrar = periodoActual - tiempoPasado;
          //lcd.print(String(aMostrar));
          lcd.setRGB(azul[0], azul[1], azul[2]);
          digitos_mostrar(aMostrar);
          comunicarEstadoActual(true, aMostrar);
        } else {
          lcd.print("Public: ");
          lcd.print(String(publicoActual));
        }


      }

    }

  }
  cambiarEstado(estadoActual());
}

void evaluarBotonSET() {
  if (estadoActual() == estado1_SELECCIONAR_N_PARTICIPANTES) {
    nParticipantes = nParticipantes + 1;
    if (nParticipantes == 6) {
      nParticipantes = 2;
    }
    evaluarEstado();
  } else if (estadoActual() == estado2_ELIJA_TIEMPO) {
    nDuracion = nDuracion + 1;
    if (nDuracion == 7) {
      nDuracion = 1;
    }

    evaluarEstado();
  } else if (estadoActual() == estado3_SELECCIONANDO_ACTOR) {
    if (participanteActual == (nParticipantes + 1)) {
      participanteActual = 0;
    } else {
      participanteActual++;
    }
    evaluarEstado();
  }
  else if (estadoActual() == estado4_ESPERAR_COMANDO) {
    if (comandoActual == comando_OK) {
      comandoActual = comando_CANCELAR;
    } else if (comandoActual == comando_CANCELAR) {
      comandoActual = comando_OK;
    }
    evaluarEstado();
  }
}

void evaluarBotonNEXT() {
  if (estadoActual() == estado1_SELECCIONAR_N_PARTICIPANTES) {
    avanzarEstado();
  } else if (estadoActual() == estado2_ELIJA_TIEMPO) {
    avanzarEstado();
  } else if (estadoActual() == estado3_SELECCIONANDO_ACTOR) {
    avanzarEstado();
    comandoActual = comando_OK;
  } else if (estadoActual() == estado4_ESPERAR_COMANDO) {
    if (comandoActual == comando_OK) {
      iniciarCuenta();
    } else {
      seleccionarActor();
    }
  } else if (estadoActual() == estado5_CUENTA_ATRAS) {
    if (tiempoLibreQ() || enPublicoQ() ) {
      seleccionarActor();
    }

  }
  evaluarEstado();
}

void setup() {
  // put your setup code here, to run once:
  randomSeed(analogRead(1));
  lcd.begin(16, 2);
  lcd.setRGB(azul[0], azul[1], azul[2]);
  lcd.setCursor(0, 0);
  lcd.print("PUCP MTR 2019 S1");
  lcd.setCursor(0, 1);
  lcd.print("Cargando...!");
  delay(3000);
  pinMode(botonSET, INPUT);  // sets the digital pin 13 as output
  pinMode(botonNEXT, INPUT);
  pinMode(salidaControl, OUTPUT);

  estadoBotonSET_pre = digitalRead(botonSET);
  estadoBotonNEXT_pre = digitalRead(botonNEXT);
  evaluarEstado();


  seg7Display.setBrightness(0x0F);
  seg7Display.clear();
  //DISPOSITIVO DE COMUNICACIONES
  radio.begin();
  radio.openWritingPipe(direccionRF);
  comunicarEstadoActual(false, 0);
  Serial.begin(115200);
}

void loop() {

  estadoBotonSET = digitalRead(botonSET);
  estadoBotonNEXT = digitalRead(botonNEXT);
  digitalWrite(salidaControl, estadoBotonSET);

  if (estadoBotonSET != estadoBotonSET_pre) {
    if (estadoBotonSET == HIGH) {
      evaluarBotonSET();
    }
    estadoBotonSET_pre = estadoBotonSET;
  }
  if (estadoBotonNEXT != estadoBotonNEXT_pre) {
    if (estadoBotonNEXT == HIGH) {
      evaluarBotonNEXT();
    }
    estadoBotonNEXT_pre = estadoBotonNEXT;
  }
  tiempoActual = now();
  if (tiempoActual != tiempoAnterior) {

    tiempoAnterior = tiempoActual;
  }
  evaluarEstado();
  delay(100);
}