#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//Crear el objeto lcd  dirección  0x3F y 16 columnas x 2 filas
LiquidCrystal_I2C lcd(0x27, 16, 2);

//TRANSCEPTOR
//Declaremos los pines CE y el CSN
#define CE_PIN 9
#define CSN_PIN 10
//Variable con la dirección del canal que se va a leer
byte direccion[5] = {'c', 'a', 'n', 'a', 'l'};
//creamos el objeto radio (NRF24L01)
RF24 radio(CE_PIN, CSN_PIN);
//vector para los datos recibidos
byte datos[7];


//CONTROL DE USUARIO
#define ledPIN  3

//SWITCH CAMBIA OPCIONES
#define BOTON  2
int val;

void setup() {
  radio.begin();  //inicializamos el NRF24L01
  Serial.begin(9600);    //inicializamos el puerto serie
  //Abrimos el canal de Lectura
  radio.openReadingPipe(1, direccion);
  //empezamos a escuchar por el canal
  radio.startListening();
  //PINES DECLARADOS PARA OPERAR
  pinMode(ledPIN , OUTPUT);  //definir pin como salida
  // Inicializar el LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  // Escribimos el Mensaje en el LCD.
  pinMode(BOTON, INPUT_PULLUP); // y BOTON como señal de entrada
  val = digitalRead(BOTON);
}

void loop() {
  //CASO ALTAVOZ
  if (val == 0 ) {
    digitalWrite(ledPIN, HIGH); //SIMULA EL ALTAVOZ
    //Pantalla
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("MODO");
    lcd.setCursor(5, 1);
    lcd.print("ALTAVOZ");
    val = digitalRead(BOTON);
    delay(100);
  }
  //CASO DEBATE
  else if (val == 1) {
    digitalWrite(ledPIN, LOW); //SIMULA EL ALTAVOZ
    //Actualización del modo de operación
    if ( radio.available() ) {
      radio.read(datos, sizeof(datos));
    }
    //caso del modo de operacion inoperaivo
    while (datos[1] == 0) {
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("MODO");
      lcd.setCursor(3, 1);
      lcd.print("DEBATE OFF");
      val = digitalRead(BOTON);
      if (val == 0) {
        break;
      }
      //Actualización del modo de operación
      else if ( radio.available() ) {
        radio.read(datos, sizeof(datos));
        break;
      }
    }
    //caso del modo de operacion operativo
    while (datos[1] = 0 && datos[2] == datos[3]) {
      digitalWrite(ledPIN, LOW); //SIMULA EL ALTAVOZ
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("DEBATE ON");
      lcd.setCursor(1, 1);
      lcd.print("ALUMNO ");
      int centena = datos[4] * 100;
      int decena = datos[5] * 10;
      int unidad = datos[6];
      int alumno = centena + decena + unidad;
      lcd.setCursor(8, 1);
      lcd.print(alumno);
      //Actualización del modo de operación
      if (val == 0) {
        break;
      }
      else if ( radio.available() ) {
        radio.read(datos, sizeof(datos));
        break;
      }
    }
    delay(100);
  }
}
