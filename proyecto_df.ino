#include <DFRobotDFPlayerMini.h>
#include "NewPing.h"

HardwareSerial DFSerial(1);
DFRobotDFPlayerMini df;
#define DF_RX   1   // ESP32-C3 RX  <- DF TX
#define DF_TX   0   // ESP32-C3 TX  -> DF RX
//#define DF_BUSY 3   // Lee el pin BUSY del DFPlayer

#define TRIGGER_PIN 4
#define ECHO_PIN 9
#define MAX_DISTANCE 100

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
unsigned long ultimaDetec = 0;
const unsigned long Tmediciones = 300;
int distancia = 0;

const int POT_Vol_PIN = 2;
const int POT_Dur_PIN = 3;

// Rango del pote Volumen
const int VALOR_MIN = 880;
const int VALOR_MAX = 4095;
// Volumen máximo del DFPlayer
const int VOLUMEN_MAX = 30;
// Parámetros de detección de cambio
const unsigned long INTERVALO_CAMBIO = 300; // ms
const int UMBRAL_CAMBIO = 60;               // cambio mínimo en analogRead
int ultimoValor = 0;
unsigned long ultimoChequeo = 0;

struct Nota {
  const char* nombre;
  int archivo;
};

// Lista de notas base
const Nota notasBase[12] = {
  {"C", 1}, {"C#", 2}, {"D", 3}, {"D#", 4}, {"E", 5}, {"F", 6},
  {"F#", 7}, {"G", 8}, {"G#", 9}, {"A", 10}, {"A#", 11}, {"B", 12}
};

// Patrones de escala
const int patronMayor[5] = {0, 2, 4, 7, 9};
const int patronMenor[5] = {0, 3, 5, 7, 10};

int archivos_notas[5];   // guarda los números de archivo q debe luego asociar a las distancias
int archivos_len = 0;

// --- CONTROL DE REPRODUCCIÓN SIN delay ---
unsigned long ultimoPlay = 0;
const unsigned long duracion = 1000;
int notaActual = -1;

int buscarNotaBase(const char* nombre) {
  for (int i = 0; i < 12; i++) {
    if (strcmp(notasBase[i].nombre, nombre) == 0)
      return notasBase[i].archivo;
  }
  return -1; // no encontrada
}

void Escala(const char* nombre, bool esMayor) {
  int base = buscarNotaBase(nombre);
  if (base == -1) {
    Serial.println("Nota base no encontrada");
    return;
  }
  const int* patron = esMayor ? patronMayor : patronMenor; //si se ingreso True, toma el patronMayor, sino toma el patronMenor
  for (int i = 0; i < 5; i++) {
    int notaArchivo = base + patron[i];
    // Para evitar pasarse de 12 notas, vuelve a empezar:
    //if (notaArchivo > 12) notaArchivo -= 12;

    archivos_notas[archivos_len++] = notaArchivo;
    Serial.print(notaArchivo);

  }
  Serial.println();
}

int altura_nota(float distancia, const int* archivos) {
  if (distancia > 4 && distancia <= 11) return archivos[0];
  if (distancia > 11 && distancia <= 18) return archivos[1];
  if (distancia > 18 && distancia <= 25) return archivos[2];
  if (distancia > 25 && distancia <= 32) return archivos[3];
  if (distancia >= 32 && distancia <= 39) return archivos[4];
  if (distancia == 0) {
    return 0;
  }

}

void setup() {
  Serial.begin(115200);
  delay(300);

  //pinMode(DF_BUSY, INPUT_PULLUP);   // si flota, añadí 10k a 3V3 externamente

  DFSerial.begin(9600, SERIAL_8N1, DF_RX, DF_TX);
  delay(300);

  if (!df.begin(DFSerial, /*isACK*/ false, /*doReset*/ true)) {
    Serial.println("No se inicializó DFPlayer (sin ACK).");
    while (true) delay(100);
  }

  df.setTimeOut(500);
  df.volume(30);
  df.reset();
  delay(1500);
  df.outputDevice(DFPLAYER_DEVICE_SD);
  delay(200);

  pinMode(POT_Vol_PIN, INPUT);
  pinMode(POT_Dur_PIN, INPUT);

  Escala("D", true); //archivos_notas se actualiza

}

void loop() {
  unsigned long ahora = millis();
  if (ahora - ultimaDetec >= Tmediciones ) {
    ultimaDetec = ahora;
    distancia = sonar.ping_cm();
    Serial.println("Distancia: ");
    Serial.print(distancia);
  }
  //int distance = 6;
  int nota_reproducir = altura_nota(distancia, archivos_notas);

  int duracionArchivo = 1;
  int duracion = 1000;
  int medida = analogRead(POT_Dur_PIN);

  if(medida >= 0 && medida <= 1300) { //si el pote esta en rango inicial, dura 1s elsample
    duracionArchivo = 1;
    duracion = 1000;
  }
  if(medida >= 1300 && medida <= 3000) { //si el pote esta en rango inicial, dura 2s elsample
    duracionArchivo = 2;
    duracion = 2000;
  }
  if(medida >= 3000 && medida <= 4095) {//si el pote esta en rango inicial, dura 4s elsample
    duracionArchivo = 3;
    duracion = 4000;
  }

  // Reproduce una nota si ya paso el tiempo de reproduccion
  if (ahora - ultimoPlay >= duracion) {
    //notaActual = nota_reproducir;
    ultimoPlay = ahora;
    if (nota_reproducir == 0) {
      df.stop();
    }
    else {
      df.playFolder(nota_reproducir, duracionArchivo);  // ajustá el orden si usás carpeta fija
    Serial.print("Reproduciendo nota: ");
    Serial.println(nota_reproducir);
    }
    
  }

  //CONTROL DE VOLUMEN
  int lectura = analogRead(POT_Vol_PIN);
  //cada 300ms chequea si hubo un cambio real en el pote
  if (ahora - ultimoChequeo >= INTERVALO_CAMBIO) {
    int diferencia = lectura - ultimoValor;
    if (diferencia < 0) diferencia = -diferencia; // abs a mano

    if (diferencia > UMBRAL_CAMBIO) {
      // Regla de 3 simple para pasar de lectura a volumen (0–30)
      int volumen = (lectura - VALOR_MIN) * VOLUMEN_MAX / (VALOR_MAX - VALOR_MIN);

      // Limitar por las dudas a 0–30
      if (volumen < 0) volumen = 0;
      if (volumen > VOLUMEN_MAX) volumen = VOLUMEN_MAX;

      // Mandar el volumen al DFPlayer
      df.volume(volumen);

      Serial.print("Lectura: ");
      Serial.print(lectura);
      Serial.print(" Volumen DFPlayer: ");
      Serial.println(volumen);
    }

    // Actualizo referencias para proxima lectura
    ultimoValor = lectura;
    ultimoChequeo = ahora;
  }

}