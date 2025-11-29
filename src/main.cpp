/* Proyecto Instrumentación Biomédica - Lectura Dual MPU6050 (Fémur y Tibia)
 * * Configuración de Hardware:
 * - Sensor Fémur: Pin AD0 conectado a GND (Dirección 0x68)
 * - Sensor Tibia: Pin AD0 conectado a 3.3V (Dirección 0x69)
 * - Ambos sensores comparten pines SDA (GPIO 21) y SCL (GPIO 22)
 */
#include "Wire.h"
#include <MPU6050_light.h>

// Definición de pines y objetos
MPU6050 mpuFemur(Wire);
MPU6050 mpuTibia(Wire);

// Variables para almacenar la postura neutra
// Guardamos el ángulo que tienen los sensores cuando el paciente está recto.
float offsetFlexion = 0;
float offsetValgo = 0;
float offsetRotacion = 0;

bool calibrado = false;
unsigned long timer = 0;

// --- Normalizar ángulos a -180/180 ---
float normalizeAngle(float angle) {
  while (angle > 180) angle -= 360;
  while (angle < -180) angle += 360;
  return angle;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Inicialización de Sensores
  // mpuFemur usa dirección 0x68 (AD0 -> GND)
  // mpuTibia usa dirección 0x69 (AD0 -> 3.3V)
  byte status = mpuFemur.begin();
  Serial.print(F("Fémur status: ")); Serial.println(status);
  
  mpuTibia.setAddress(0x69);
  status = mpuTibia.begin();
  Serial.print(F("Tibia status: ")); Serial.println(status);

  Serial.println(F("CALIBRANDO GIROSCOPIOS... NO MOVER"));
  delay(1000);
  mpuFemur.calcOffsets(true, true);
  mpuTibia.calcOffsets(true, true);
  Serial.println(F("¡Listo! Coloque al paciente en POSICIÓN NEUTRA (De pie, recto)."));
  Serial.println(F("Envíe 'c' por el monitor serie para establecer el CERO clínico."));
}

void loop() {
  // Actualizar datos de sensores (necesario siempre)
  mpuFemur.update();
  mpuTibia.update();

  // --- ESCUCHAR COMANDO DE CALIBRACIÓN ---
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'c') {
      
      // 1. Obtenemos los ángulos actuales del sensor
      float femurPitch = mpuFemur.getAngleY(); // Asumiendo Y como eje de flexión
      float tibiaPitch = mpuTibia.getAngleY();
      
      float femurRoll = mpuFemur.getAngleX();  // Asumiendo X como eje Var/Val
      float tibiaRoll = mpuTibia.getAngleX();
      
      // 2. Calculamos la diferencia actual y la guardamos como offset
      offsetFlexion = tibiaPitch - femurPitch;
      offsetValgo   = tibiaRoll - femurRoll;
      
      calibrado = true;
      Serial.println(F(">>> CALIBRACIÓN CLÍNICA COMPLETADA <<<"));
    }
  }

  // --- CÁLCULO DE ÁNGULOS CLÍNICOS ---
  if (calibrado && millis() - timer > 100) {
    
    // 1. Obtener ángulos absolutos
    float fP = mpuFemur.getAngleY();
    float tP = mpuTibia.getAngleY();
    
    float fR = mpuFemur.getAngleX();
    float tR = mpuTibia.getAngleX();

    // 2. Calcular Ángulo RELATIVO (Articulación)
    // Fórmula: (Tibia - Fémur) - Offset_Inicial
    float flexion = (tP - fP) - offsetFlexion;
    float valgo   = (tR - fR) - offsetValgo;

    // 3. Normalizar (evitar saltos de 360 grados)
    flexion = normalizeAngle(flexion);
    valgo = normalizeAngle(valgo);

    // 4. Mostrar Datos (formato CSV)
    Serial.print(flexion, 1);
    Serial.print("\t\t| ");
    Serial.print(valgo, 1);
    Serial.print("\t\t");

    timer = millis();
  } else if (!calibrado && millis() - timer > 1000) {
    Serial.println("Esperando calibración (escriba 'c')...");
    timer = millis();
  }
}