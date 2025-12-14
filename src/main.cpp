/* Proyecto Instrumentación Biomédica - Lectura Dual MPU6050 (Fémur y Tibia)
 * * Configuración de Hardware:
 * - Sensor Fémur: Pin AD0 conectado a GND (Dirección 0x68)
 * - Sensor Tibia: Pin AD0 conectado a 3.3V (Dirección 0x69)
 * - Ambos sensores comparten pines SDA (GPIO 21) y SCL (GPIO 22)
 */
#include "Wire.h"
#include <MPU6050_light.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Definición de pines y objetos
MPU6050 mpuFemur(Wire);
MPU6050 mpuTibia(Wire);

// Variables para almacenar la postura neutra
// Guardamos el ángulo que tienen los sensores cuando el paciente está recto.
float offsetFlexion = 0;
float offsetValgo = 0;

bool calibrado = false;
bool transmitiendo = false;

unsigned long timer = 0;

// --- Normalizar ángulos a -180/180 ---
float normalizeAngle(float angle) {
  while (angle > 180) angle -= 360;
  while (angle < -180) angle += 360;
  return angle;
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("IMB_MPU6050_Dual"); // Nombre del dispositivo Bluetooth
  Serial.println(F(">>> INSTRUMENTACIÓN BIOMÉDICA - LECTURA DUAL MPU6050 <<<"));

  Wire.begin(); 
  Wire.setClock(400000); // Configurar I2C a 400kHz
  
  // Inicialización de Sensores
  // mpuFemur usa dirección 0x68 (AD0 -> GND)
  // mpuTibia usa dirección 0x69 (AD0 -> 3.3V)
  byte status = mpuFemur.begin();
  Serial.print(F("Fémur status: ")); Serial.println(status);
  
  mpuTibia.setAddress(0x69);
  status = mpuTibia.begin();
  Serial.print(F("Tibia status: ")); Serial.println(status);

  Serial.println(F("CALIBRANDO OFFSETS... NO MOVER"));
  mpuFemur.calcOffsets(true, true);
  mpuTibia.calcOffsets(true, true);

  Serial.println(F("SISTEMA LISTO. Comandos: 'c'=Calibrar, 'i'=Iniciar, 's'=Detener"));
}

void loop() {
  // Actualizar datos de sensores (necesario siempre)
  mpuFemur.update();
  mpuTibia.update();

  if (SerialBT.available()) {
    char command = (char)SerialBT.read();

    switch (command) {
      case 'c':
        // 1. Obtenemos los ángulos actuales del sensor
        offsetFlexion = mpuTibia.getAngleY() - mpuFemur.getAngleY();
        offsetValgo   = mpuTibia.getAngleX() - mpuFemur.getAngleX();
        calibrado = true;
        SerialBT.println("OK: Calibracion Clinica Completada.");
        break;

      case 'i':
        if (calibrado) {
          transmitiendo = true;
          SerialBT.println(F(">>> TRANSMISIÓN INICIADA <<<"));
          SerialBT.println("Tiempo(ms),Flexion,Valgo"); // Cabecera de datos CSV
          break;
        } else {
          SerialBT.println(F("ERROR: Primero calibrar con 'c'"));
          break;
        }

      case 's':
        transmitiendo = false;
        SerialBT.println(F(">>> TRANSMISIÓN DETENIDA <<<"));
        break;

      default:
        SerialBT.println(F("Comando no reconocido. Use 'c', 'i' o 's'."));
        break;
    }
  }

  if (transmitiendo && millis() - timer > 50) { // 20 Hz (Ajustable)
      
      float fP = mpuFemur.getAngleY();
      float tP = mpuTibia.getAngleY();
      float fR = mpuFemur.getAngleX();
      float tR = mpuTibia.getAngleX();

      float flexion = normalizeAngle((tP - fP) - offsetFlexion);
      float valgo   = normalizeAngle((tR - fR) - offsetValgo);

      // Formato CSV (Comma Separated Values)
      // Esto permite guardar el log en el cel y abrirlo directo en Excel/Matlab
      SerialBT.print(millis());
      SerialBT.print(",");
      SerialBT.print(flexion, 1);
      SerialBT.print(",");
      SerialBT.println(valgo, 1);

      timer = millis();
    }
}