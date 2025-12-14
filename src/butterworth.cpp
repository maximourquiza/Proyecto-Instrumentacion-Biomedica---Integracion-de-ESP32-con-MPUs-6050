// ==========================================
// CLASE FILTRO BUTTERWORTH 2DO ORDEN
// ==========================================
#include <math.h>

#ifndef M_PI
#   define M_PI 3.1415926535897932384626433832
#endif

class LowPassFilter {
  private:
    // Coeficientes (se calculan según Fs y Fc)
    float a1, a2;
    float b0, b1, b2;

    // Buffers de historia (n-1, n-2)
    float x1 = 0, x2 = 0; // Inputs anteriores
    float y1 = 0, y2 = 0; // Outputs anteriores
    
    static const float PI;

  public:
    // Constructor vacío
    LowPassFilter() {}

    // Inicializador: Configura los coeficientes
    // fs: Frecuencia de muestreo (ej. 100 Hz)
    // fc: Frecuencia de corte deseada (ej. 10 Hz)
    void begin(float fs, float fc) {
      float omega = 2.0f * M_PI * (fc / fs);
      float sn = sin(omega);
      float cs = cos(omega);
      float alpha = sn / (2.0f * 0.707f); // 0.707 es 1/sqrt(2) para Butterworth

      float a0 = 1.0f + alpha;
      
      // Fórmulas estándar para Biquad Low Pass
      b0 = (1.0f - cs) / 2.0f / a0;
      b1 = (1.0f - cs) / a0;
      b2 = (1.0f - cs) / 2.0f / a0;
      a1 = (-2.0f * cs) / a0;
      a2 = (1.0f - alpha) / a0;
    }

    // Función principal: Filtra un nuevo dato
    float filter(float input) {
      // Ecuación en diferencias
      float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

      // Desplazar historia (Shift)
      x2 = x1;
      x1 = input;
      y2 = y1;
      y1 = output;

      return output;
    }
    
    // Resetea el filtro (útil si hay un salto brusco o recalibración)
    void reset() {
      x1 = 0; x2 = 0;
      y1 = 0; y2 = 0;
    }
};
