

#include "Wire.h"
#include <MPU6050_light.h>
#include "BluetoothSerial.h"
#include <Arduino.h>
#include "LowPassFilter.h"

BluetoothSerial SerialBT;
LowPassFilter lpFlex;
LowPassFilter lpValgo;
MPU6050 mpuFemur(Wire);
MPU6050 mpuTibia(Wire);

// Offsets para la "Tara" anatómica (cuando el paciente está parado)
float offFlexFemur = 0, offValgoFemur = 0;
float offFlexTibia = 0, offValgoTibia = 0;

bool calibradoAnatomico = false; // Bandera para saber si ya hicimos la "Tara"
bool transmitiendo = false;

unsigned long timer = 0;

// Normalización de ángulos
float normalizeAngle(float angle) {
  while (angle > 180) angle -= 360;
  while (angle < -180) angle += 360;
  return angle;
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("IMB_MPU6050_Dual"); 

  Serial.println(F(">>> INICIANDO SISTEMA DUAL <<<"));
  
  Wire.begin(); // Forzamos pines SDA, SCL
  Wire.setClock(400000); 

  // --- INICIALIZACIÓN DE SENSORES ---
  
  // 1. MPU FEMUR (0x68)
  byte status = mpuFemur.begin();
  Serial.print(F("Status Femur (0x68): ")); Serial.println(status);
  
  // 2. MPU TIBIA (0x69)
  mpuTibia.setAddress(0x69); 
  status = mpuTibia.begin();
  Serial.print(F("Status Tibia (0x69): ")); Serial.println(status);

  // --- CALIBRACIÓN DE GIROSCOPIOS (HARDWARE) ---
  Serial.println(F("ATENCION: MANTENER SENSORES PLANOS Y QUIETOS EN LA MESA"));
  Serial.println(F("Calibrando en 3 segundos..."));
  delay(3000);
  
  Serial.println(F("Calculando Offsets Hardware... NO TOCAR"));
  mpuFemur.calcOffsets(true, true); // Calibra Gyro y Accel
  mpuTibia.calcOffsets(true, true);
  
  Serial.println(F("Hardware Listo. Coloca los sensores en el paciente."));
  Serial.println(F("Comandos: 'c' (Estando parado para tara), 'i' (Iniciar), 's' (Stop)"));
}

void loop() {
  mpuFemur.update();
  mpuTibia.update();

  if (SerialBT.available()) {
    char command = (char)SerialBT.read();

    switch (command) {
      case 'c': {
        SerialBT.println("Calibrando Postura Inicial (Tara)... manténgase quieto.");
        float sumFF = 0, sumVF = 0, sumFT = 0, sumVT = 0;
        int n = 100; // 100 muestras

        for(int i=0; i<n; i++) {
            mpuFemur.update();
            mpuTibia.update();
            
            sumFF += mpuFemur.getAngleY(); 
            sumVF += mpuFemur.getAngleX();
            sumFT += mpuTibia.getAngleY(); 
            sumVT += mpuTibia.getAngleX();
            
            delay(5);
        }

        offFlexFemur = sumFF / n;
        offValgoFemur = sumVF / n;
        offFlexTibia = sumFT / n;
        offValgoTibia = sumVT / n;
        
        calibradoAnatomico = true;
        SerialBT.println("OK: Cero Anatomico Establecido.");
        Serial.println("Calibracion 'c' exitosa.");
        break;
      }

      case 'i':
        if (calibradoAnatomico) {
          transmitiendo = true;
          SerialBT.println("TIEMPO,FLEXION,VALGO"); // Header CSV
        } else {
          SerialBT.println("ERROR: Presione 'c' primero estando parado.");
        }
        break;

      case 's':
        transmitiendo = false;
        SerialBT.println("STOP");
        break;
    }
  }

  // 3. ENVÍO DE DATOS Fm = 100 Hz
  if (transmitiendo && millis() - timer > 10) {
      
      // Calculamos ángulos restando el offset inicial (Tara)
      float fP = mpuFemur.getAngleY() - offFlexFemur;
      float tP = mpuTibia.getAngleY() - offFlexTibia;
      
      float fR = mpuFemur.getAngleX() - offValgoFemur;
      float tR = mpuTibia.getAngleX() - offValgoTibia;

      // Calculamos la diferencia entre Tibia y Fémur
      float flexion = normalizeAngle(fP - tP);
      float valgo   = normalizeAngle(fR - tR);

      float lpf_flex = lpFlex.filter(flexion);
      float lpf_valgo = lpValgo.filter(valgo);

      // --- ENVIAR A BLUETOOTH (Formato CSV para App/Excel) ---
      SerialBT.print(millis());
      SerialBT.print(",");
      SerialBT.print(lpf_flex, 1);
      SerialBT.print(",");
      SerialBT.println(lpf_valgo, 1);

      timer = millis();
  }
}