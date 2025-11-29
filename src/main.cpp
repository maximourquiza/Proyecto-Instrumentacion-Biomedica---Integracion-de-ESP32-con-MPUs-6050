/* Proyecto Instrumentación Biomédica - Lectura Dual MPU6050 (Fémur y Tibia)
 * * Configuración de Hardware:
 * - Sensor Fémur: Pin AD0 conectado a GND (Dirección 0x68)
 * - Sensor Tibia: Pin AD0 conectado a 3.3V (Dirección 0x69)
 * - Ambos sensores comparten pines SDA (GPIO 21) y SCL (GPIO 22)
 */
#include "Wire.h"
#include <MPU6050_light.h>

// 1. Instanciamos dos objetos MPU6050 independientes
// Ambos utilizan el mismo bus I2C (Wire)
MPU6050 mpuFemur(Wire);
MPU6050 mpuTibia(Wire);

unsigned long timer = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // 2. Configuración del Sensor FEMUR (0x68)
  
  byte statusFemur = mpuFemur.begin();
  Serial.print(F("Estado MPU Fémur (0x68): "));
  Serial.println(statusFemur);
  
  // 3. Configuración del Sensor TIBIA (0x69)
  mpuTibia.setAddress(0x69); 
  
  byte statusTibia = mpuTibia.begin();
  Serial.print(F("Estado MPU Tibia (0x69): "));
  Serial.println(statusTibia);

  // Verificación de seguridad: detener si alguno falla
  while(statusFemur != 0 || statusTibia != 0){
    Serial.println(F("¡Error al conectar sensores! Revisa cables y direcciones AD0."));
    delay(1000);
  }
  
  // 4. Calibración
  // Los sensores deben estar absolutamente quietos y planos
  Serial.println(F("Calibrando offsets... NO MOVER LOS SENSORES"));
  delay(1000);
  
  Serial.println(F("Calibrando Fémur..."));
  mpuFemur.calcOffsets(true, true);
  
  Serial.println(F("Calibrando Tibia..."));
  mpuTibia.calcOffsets(true, true);
  
  Serial.println("¡Calibración completada!\n");
  Serial.println("Fémur X\t\tTibia X\t\t| Fémur Y\tTibia Y"); // Encabezado
  
  timer = millis();
}

void loop() {
  // 5. Actualizar lecturas de ambos sensores
  // Es necesario llamar a update() en cada ciclo para que el filtro interno funcione
  mpuFemur.update();
  mpuTibia.update();

  if(millis() - timer > 100){ // Imprimir cada 100ms
    
    // Obtenemos los ángulos de interés (ej. Flexión/Extensión que suele ser el Eje X o Y según montaje)
    float anguloFemurX = mpuFemur.getAngleX();
    float anguloTibiaX = mpuTibia.getAngleX();
    
    float anguloFemurY = mpuFemur.getAngleY();
    float anguloTibiaY = mpuTibia.getAngleY();

    float anguloFemurZ = mpuFemur.getAngleZ();
    float anguloTibiaZ = mpuTibia.getAngleZ();

    // Visualización por Serial
    Serial.print(anguloFemurX, 1);
    Serial.print("\t\t");
    Serial.print(anguloTibiaX, 1);
    Serial.print("\t\t| ");
    Serial.print(anguloFemurY, 1);
    Serial.print("\t\t");
    Serial.print(anguloTibiaY, 1);
    Serial.print("\t\t| ");
    Serial.print(anguloFemurZ, 1);
    Serial.print("\t\t");
    Serial.print(anguloTibiaZ, 1);
    
    Serial.println();
    timer = millis();
  }
}