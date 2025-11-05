/* Get all possible data from MPU6050
 * Accelerometer values are given as multiple of the gravity [1g = 9.81 m/s²]
 * Gyro values are given in deg/s
 * Angles are given in degrees
 * Note that X and Y are tilt angles and not pitch/roll.
 *
 * License: MIT
 */

#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

// Variables para análisis de calidad del filtro
float gyro_x_raw = 0, gyro_y_raw = 0, gyro_z_raw = 0;
float accel_x_raw = 0, accel_y_raw = 0, accel_z_raw = 0;

unsigned long timer = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(true,true); // gyro and accelero
  Serial.println("Done!\n");

  timer = millis();
}

void loop() {

  mpu.update();

  float angleX = mpu.getAngleX();  // Roll - Inclinación lateral
  float angleY = mpu.getAngleY();  // Pitch - Inclinación adelante/atrás
  float angleZ = mpu.getAngleZ();  // Yaw - Rotación horizontal (deriva sin magnetómetro)

  // Estos son valores RAW con offsets ya aplicados
  float accelX = mpu.getAccX();  // Aceleración en X (en g's)
  float accelY = mpu.getAccY();  // Aceleración en Y (en g's)
  float accelZ = mpu.getAccZ();  // Aceleración en Z (en g's)
  
  float gyroX = mpu.getGyroX();  // Velocidad angular en X (°/s)
  float gyroY = mpu.getGyroY();  // Velocidad angular en Y (°/s)
  float gyroZ = mpu.getGyroZ();  // Velocidad angular en Z (°/s)

  // Detectar si el sensor está quieto o en movimiento
  // Calculamos la magnitud del vector de aceleración
  float accelMagnitude = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
  bool enMovimiento = abs(accelMagnitude - 1.0) > 0.15;  // >0.15g de diferencia
  
  // Detectar velocidades angulares significativas
  float gyroMagnitude = sqrt(gyroX*gyroX + gyroY*gyroY + gyroZ*gyroZ);
  bool rotacionRapida = gyroMagnitude > 30.0;  // >30°/s

  if(millis() - timer > 100){ // print data every second
    // Ángulos filtrados
    Serial.print(angleX, 2);
    Serial.print("\t\t");
    Serial.print(angleY, 2);
    Serial.print("\t\t");
    Serial.print(angleZ, 2);
    Serial.print("\t| ");
    
    // Aceleraciones
    Serial.print(accelX, 3);
    Serial.print("\t\t");
    Serial.print(accelY, 3);
    Serial.print("\t\t");
    Serial.print(accelZ, 3);
    Serial.print("\t| ");
    
    // Velocidades angulares
    Serial.print(gyroX, 2);
    Serial.print("\t\t");
    Serial.print(gyroY, 2);
    Serial.print("\t\t");
    Serial.print(gyroZ, 2);
    
    // Indicadores de estado
    if (enMovimiento) {
      Serial.print("\t[MOV]");
    }
    if (rotacionRapida) {
      Serial.print("\t[ROT]");
    }
    
    Serial.println();
    timer = millis();
  }

}