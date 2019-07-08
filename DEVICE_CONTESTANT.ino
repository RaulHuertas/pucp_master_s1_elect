/*Este es el protocolo a usar para sincronizar los equipos
  - El equipo del moderador es el master y el resto son los esclavos
  - Habrá n participantes
  - El número total de equipos es n+2 (uno por participante, moderador y público)
  - El dispositivo moderador es el que se configura para seleccionar
  cuantos dspositivos van a haber, cuanto tiempo va a tener cada uno
  y a quién le toca un turno.
  - Cada 100 ms, el equipo moderador enviará una trama que indique el
  estado actual del sistema, indicando que dispositivo es el activo
  y el tiempo que debe mostrar en su pantalla.
  - Cada dispositivo de los participantes ha de tener un menu en
  el que se seleccione a qué número de participante corresponde (1, 2, 3...).
  El dispositivo para el público no necesita este menú, sólo tiene un único rol.

  Trama a enviar(tamaño total: 7 bytes):
  - 0: cabecera (0xFF) : Para que los dispositivos sepan que estan recibiendo una trama nueva
  - 1; participandoQ: El master indica si hay algun participante
  - 2: nParticipantes: El numero de participantes en al red
  - 3: dispositivoActivo: 0-(n-1) -> Dispositivo de participante, n -> Dispositivo del público
  - 4-6: segundosRestantes, en bcd (centenas, decenas, unidades)

  INTERPRETACION DE ESTOS DATOS EN DISPOSITIVOS DE PARTICIPANTES:
  - Luego de que al dispositivo se le ha configurado su participante, N(1-n),
  debe empezar a recibir estas tramas
  - Si recibe una trama con el byte 'participandoQ' en 1 y el byte 'dispositivoActivo' igual a
  (N-1), entonces debe mostrar el valor de segundosRestantes en su pantalla
  - En cualquier otra condicion, debe mostrar 0000 en su pantalla.

  INTERPRETACION DE ESTOS DATOS EN DISPOSITIVOS DE PUBLICO:
  - Si recibe una trama con el byte 'participandoQ' en 1 y el byte 'dispositivoActivo' igual a
  n(dispositivoActivo==nParticipantes), entonces debe mostrar el valor de segundosRestantes
  en su pantalla
  - En cualquier otra condicion, debe mostrar 0000 y apagar su microfono.

  CANAL RF:
  byte direccionRF[5] ={'p','u','c','p','0'};

  Ejemplo de trama:
  0: 0xFF
  1: 0x01 -> hay participacion
  2: 0x02 -> hay dos participantes
  3: 0x01 -> Participante 2 activo, el resto ha de tener 0 en sus pantallas
  4: 0x00 ->
  5: 0x02 ->  27 segundos restantes
  6: 0x07 ->

  REFERENCIA:
  https://naylampmechatronics.com/blog/16_Tutorial-b%C3%A1sico-NRF24L01-con-Arduino.html
*/
//PESUDO-CODIGO PARA INTERPRETAR LA TRAMA EN LOS DISPOSITIVOS ESCLAVOS
//al principio del sketch

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); //

//Para el NRF24L01
//Declaremos los pines CE y el CSN
#define CE_PIN 9
#define CSN_PIN 10

//Variable con la dirección del canal que se va a leer
byte direccion[5] = {'p', 'u', 'c', 'p', '0'};

//creamos el objeto radio (NRF24L01)
RF24 radio(CE_PIN, CSN_PIN);

//vector para los datos recibidos
//float datos[3];

byte trama[7];
byte bufffer[1];
int indice = 0;
int N = 1;
//byte nuevoByte;

void setup()
{
  //inicializamos el NRF24L01
  radio.begin();
  //inicializamos el puerto serie
  Serial.begin(9600);

  //Abrimos el canal de Lectura
  radio.openReadingPipe(1, direccion);

  //empezamos a escuchar por el canal
  radio.startListening();

  lcd.init();

  //Encender la luz de fondo.
  lcd.backlight();

  // Escribimos el Mensaje en el LCD.
  lcd.setCursor(0, 0);             // Cursor a linea 1, posicion 1
  lcd.print("PUCP MTR 2019 S1");

  lcd.setCursor(0, 1);             // Cursor a linea 2, posicion 1
  lcd.print("Cargando...");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);             // Cursor a linea 1, posicion 1
  lcd.print("PUCP MTR 2019 S1");
  
  
  
}

void loop() {
  //En el 'loop' del sketch
  delay(10);
  if ( radio.available() )
  {

    radio.read(trama, 7);
    Serial.println(trama[0]);
    if (trama[0] == 0x0FF) {
      //      indice = 1;
      //      trama[0] = bufffer[0];
      //    }else if (indice < sizeof(trama)) {
      //      trama[indice] = bufffer[0];
      //      indice++;
      //      Serial.println(indice);

      Serial.print("Dato enviado 0: ");
      Serial.println(String((int)trama[0]));
      Serial.print("Dato enviado 1: ");
      Serial.println(String((int)trama[1]));
      Serial.print("Dato enviado 2: ");
      Serial.println(String((int)trama[2]));
      Serial.print("Dato enviado 3: ");
      Serial.println(String((int)trama[3]));
      Serial.print("Dato enviado 4: ");
      Serial.println(String((int)trama[4]));
      Serial.print("Dato enviado 5: ");
      Serial.println(String((int)trama[5]));
      Serial.print("Dato enviado 6: ");
      Serial.println(String((int)trama[6]));

      //if (indice == sizeof(trama)) {
      int numero = 0;
      if ( trama[1] != 0 && (N - 1) == trama[3]) {
        numero = 100 * trama[4] + 10 * trama[5] + trama[6];

      }

      Serial.println(numero);
      lcd.setCursor(0, 1);
      lcd.print(String(numero));



      //INTERPPRETAR LOS 7 BYTES DE 'trama'
      //}
    }
  }

}
