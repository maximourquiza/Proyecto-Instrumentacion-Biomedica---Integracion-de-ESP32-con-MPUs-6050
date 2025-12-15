// ==========================================
// CLASE FILTRO BUTTERWORTH 2DO ORDEN
// ==========================================
#include <math.h>

#ifndef M_PI
#   define M_PI 3.1415926535897932384626433832
#endif

#ifndef FILTROPASABAJO_H
#define FILTROPASABAJO_H

class LowPassFilter {
  private:

    // Buffers de historia (n-1, n-2)
    float x1 = 0, x2 = 0; // Inputs anteriores
    float y1 = 0, y2 = 0; // Outputs anteriores

  public:
    LowPassFilter() {}

    // Función principal: Filtra un nuevo dato
    float filter(float input) {
      float a[] = {0.82386497, -0.2942198};
      float b[] = {0.11758871, 0.23517741, 0.11758871};
      // Ecuación en diferencias
      float output = a[0] * y1 + a[1] * y2 + b[0] * input + b[1] * x1 + b[2] * x2;

      x2 = x1;
      x1 = input;
      y2 = y1;
      y1 = output;

      return output;
    }
};

#endif