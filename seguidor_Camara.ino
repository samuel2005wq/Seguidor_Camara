#include <Arduino.h>
#include <math.h>

// ========================
// DEFINICIÓN DE PINES
// ========================

// Pines generales
#define Pmalo 4
#define Boton 13
#define LED_R 2
#define LED_V 3

// Motor izquierdo
#define PWMA 11
#define AIN1 6
#define AIN2 5

// Motor derecho
#define PWMB 10
#define BIN1 9
#define BIN2 8

// Pin standby del driver
#define STBY 7

//Pin camara
#define CAM_RX 12

// ========================
// Librerías
// ========================
#include "QTR8A_vel.h"
#include "MotoresNano.h"
#include <SoftwareSerial.h>

// ========================
// QTR
// ========================
const uint8_t qtrPins[8] = { A7, A6, A5, A4, A3, A2, A1, A0 };
QTR8A qtr(qtrPins, 700);

// ========================
// Serial cámara
// ========================
SoftwareSerial camSerial(CAM_RX, 255);

// ========================
// Buffers red neuronal
// ========================
float capa1_out[8];
float capa2_out[4];
float capa3_out[4];
float final_out[2];

// ========================
// PESOS (Actualizados desde tu Colab "Untitled2.ipynb")
// ========================

// Capa Entrada (8x8)
const float W_entrada[8][8] = {
  { 0.2837897 , -1.3745161 ,  0.04805442, -0.41614547,  0.48775777, -0.18942666, -0.10740258,  0.81795543},
  { 0.1038332 , -0.10764106,  0.6593131 , -0.12037776, -0.0141713 , -0.037503  ,  0.06810559,  0.53283364},
  { 0.30733952, -0.5871939 , -0.15297858, -0.2273288 , -0.23797256,  0.36185968,  0.23624264,  0.25154975},
  {-0.1138662 , -0.34324104, -0.5637441 , -0.2692978 , -0.7770961 , -0.2170197 ,  0.06552629,  0.28909487},
  {-0.01355432,  0.05957557, -0.16529015, -0.30179292,  0.5017361 ,  0.2791509 ,  0.34927234, -0.51343703},
  {-0.17393567, -0.13682956, -0.15785746, -0.14152579,  0.23131895,  0.6459056 ,  0.3877575 , -0.92393935},
  {-0.34566906,  0.634634  , -0.44916093, -0.9167286 , -0.49680522,  0.21251321,  0.33531606, -0.13315655},
  {-0.31982622,  0.57951015, -0.5408225 ,  0.7348879 ,  0.2539443 , -0.12994844,  0.11060774, -0.5645099 }
};
const float b_entrada[8] = { 0.24399157, 0.61695, 0.17681442, 0.49620634, -0.02035307, 0.24836469, 0.54449946, 0.19965117};

// Capa Oculta A (8x4)
const float W_ocultaA[8][4] = {
  {-0.71727884,  0.04688878, -0.2350057 ,  0.08260653},
  { 1.1886091 , -0.6900985 ,  0.03345253,  0.48447677},
  {-0.2878489 ,  0.99082077,  0.51218146,  0.9851669 },
  { 1.3736029 ,  0.13738579, -0.44187388, -1.1119093 },
  {-0.25534806,  0.5682747 , -0.27275553, -0.24222921},
  { 0.34080306, -0.43790293,  0.66874677, -0.3449801 },
  { 0.16950825,  0.01522589, -0.6766022 ,  0.9184531 },
  { 0.43266323,  0.58566755, -0.35542843,  0.5067952 }
};
const float b_ocultaA[4] = { 0.33844402, 0.0799186 , -0.04385266, 0.49397948};

// Capa Oculta B (4x4)
const float W_ocultaB[4][4] = {
  {-0.6135256 ,  0.15545481,  0.01957679,  1.6220471 },
  { 0.35953182,  1.1446807 ,  0.522528  , -0.9704896 },
  { 0.2652968 ,  0.49359122,  0.67291194, -0.8573798 },
  { 0.8669941 ,  0.67684025, -0.45744604, -0.07155801}
};
const float b_ocultaB[4] = { 0.30123928, 0.24319226, -0.06701466, 0.51992214};

// Capa Salida (4x2)
const float W_salida[4][2] = {
  {-0.63332546,  1.290108  },
  {-0.9864959 , -0.49209386},
  { 0.73939717,  0.7706595 },
  { 0.7151018 , -0.70482427}
};
const float b_salida[2] = {-0.08472419, -0.21432157};

// ========================
// Funciones Matemáticas
// ========================
float sigmoid(float x) {
  return 1.0 / (1.0 + exp(-x));
}

// ========================
// Forward (SIN FOR, neurona por neurona)
// ========================
void forward(float *in) {

  // Entradas
  float X1 = in[0];
  float X2 = in[1];
  float X3 = in[2];
  float X4 = in[3];
  float X5 = in[4];
  float X6 = in[5];
  float X7 = in[6];
  float X8 = in[7];

  // ------------------------
  // CAPA 1 (8 neuronas, ReLU)
  // ------------------------
  float S1, S2, S3, S4, S5, S6, S7, S8;
  S1 = b_entrada[0] + (X1*W_entrada[0][0]) + (X2*W_entrada[1][0]) + (X3*W_entrada[2][0]) + (X4*W_entrada[3][0]) + (X5*W_entrada[4][0]) + (X6*W_entrada[5][0]) + (X7*W_entrada[6][0]) + (X8*W_entrada[7][0]);
  S2 = b_entrada[1] + (X1*W_entrada[0][1]) + (X2*W_entrada[1][1]) + (X3*W_entrada[2][1]) + (X4*W_entrada[3][1]) + (X5*W_entrada[4][1]) + (X6*W_entrada[5][1]) + (X7*W_entrada[6][1]) + (X8*W_entrada[7][1]);
  S3 = b_entrada[2] + (X1*W_entrada[0][2]) + (X2*W_entrada[1][2]) + (X3*W_entrada[2][2]) + (X4*W_entrada[3][2]) + (X5*W_entrada[4][2]) + (X6*W_entrada[5][2]) + (X7*W_entrada[6][2]) + (X8*W_entrada[7][2]);
  S4 = b_entrada[3] + (X1*W_entrada[0][3]) + (X2*W_entrada[1][3]) + (X3*W_entrada[2][3]) + (X4*W_entrada[3][3]) + (X5*W_entrada[4][3]) + (X6*W_entrada[5][3]) + (X7*W_entrada[6][3]) + (X8*W_entrada[7][3]);
  S5 = b_entrada[4] + (X1*W_entrada[0][4]) + (X2*W_entrada[1][4]) + (X3*W_entrada[2][4]) + (X4*W_entrada[3][4]) + (X5*W_entrada[4][4]) + (X6*W_entrada[5][4]) + (X7*W_entrada[6][4]) + (X8*W_entrada[7][4]);
  S6 = b_entrada[5] + (X1*W_entrada[0][5]) + (X2*W_entrada[1][5]) + (X3*W_entrada[2][5]) + (X4*W_entrada[3][5]) + (X5*W_entrada[4][5]) + (X6*W_entrada[5][5]) + (X7*W_entrada[6][5]) + (X8*W_entrada[7][5]);
  S7 = b_entrada[6] + (X1*W_entrada[0][6]) + (X2*W_entrada[1][6]) + (X3*W_entrada[2][6]) + (X4*W_entrada[3][6]) + (X5*W_entrada[4][6]) + (X6*W_entrada[5][6]) + (X7*W_entrada[6][6]) + (X8*W_entrada[7][6]);
  S8 = b_entrada[7] + (X1*W_entrada[0][7]) + (X2*W_entrada[1][7]) + (X3*W_entrada[2][7]) + (X4*W_entrada[3][7]) + (X5*W_entrada[4][7]) + (X6*W_entrada[5][7]) + (X7*W_entrada[6][7]) + (X8*W_entrada[7][7]);
  
  capa1_out[0] = max(0.0f, S1);
  capa1_out[1] = max(0.0f, S2);
  capa1_out[2] = max(0.0f, S3);
  capa1_out[3] = max(0.0f, S4);
  capa1_out[4] = max(0.0f, S5);
  capa1_out[5] = max(0.0f, S6);
  capa1_out[6] = max(0.0f, S7);
  capa1_out[7] = max(0.0f, S8);

  float Y1 = capa1_out[0];
  float Y2 = capa1_out[1];
  float Y3 = capa1_out[2];
  float Y4 = capa1_out[3];
  float Y5 = capa1_out[4];
  float Y6 = capa1_out[5];
  float Y7 = capa1_out[6];
  float Y8 = capa1_out[7];

  // ------------------------
  // CAPA 2 (4 neuronas, ReLU)
  // ------------------------
  float T1, T2, T3, T4;
  T1 = b_ocultaA[0] + Y1*W_ocultaA[0][0] + Y2*W_ocultaA[1][0] + Y3*W_ocultaA[2][0] + Y4*W_ocultaA[3][0] + Y5*W_ocultaA[4][0] + Y6*W_ocultaA[5][0] + Y7*W_ocultaA[6][0] + Y8*W_ocultaA[7][0];
  T2 = b_ocultaA[1] + Y1*W_ocultaA[0][1] + Y2*W_ocultaA[1][1] + Y3*W_ocultaA[2][1] + Y4*W_ocultaA[3][1] + Y5*W_ocultaA[4][1] + Y6*W_ocultaA[5][1] + Y7*W_ocultaA[6][1] + Y8*W_ocultaA[7][1];
  T3 = b_ocultaA[2] + Y1*W_ocultaA[0][2] + Y2*W_ocultaA[1][2] + Y3*W_ocultaA[2][2] + Y4*W_ocultaA[3][2] + Y5*W_ocultaA[4][2] + Y6*W_ocultaA[5][2] + Y7*W_ocultaA[6][2] + Y8*W_ocultaA[7][2];
  T4 = b_ocultaA[3] + Y1*W_ocultaA[0][3] + Y2*W_ocultaA[1][3] + Y3*W_ocultaA[2][3] + Y4*W_ocultaA[3][3] + Y5*W_ocultaA[4][3] + Y6*W_ocultaA[5][3] + Y7*W_ocultaA[6][3] + Y8*W_ocultaA[7][3];
  
  capa2_out[0] = max(0.0f, T1);
  capa2_out[1] = max(0.0f, T2);
  capa2_out[2] = max(0.0f, T3);
  capa2_out[3] = max(0.0f, T4);

  float Z1 = capa2_out[0];
  float Z2 = capa2_out[1];
  float Z3 = capa2_out[2];
  float Z4 = capa2_out[3];

  // ------------------------
  // CAPA 3 (4 neuronas, ReLU)
  // ------------------------
  float U1, U2, U3, U4;
  U1 = b_ocultaB[0] + Z1*W_ocultaB[0][0] + Z2*W_ocultaB[1][0] + Z3*W_ocultaB[2][0] + Z4*W_ocultaB[3][0];
  U2 = b_ocultaB[1] + Z1*W_ocultaB[0][1] + Z2*W_ocultaB[1][1] + Z3*W_ocultaB[2][1] + Z4*W_ocultaB[3][1];
  U3 = b_ocultaB[2] + Z1*W_ocultaB[0][2] + Z2*W_ocultaB[1][2] + Z3*W_ocultaB[2][2] + Z4*W_ocultaB[3][2];
  U4 = b_ocultaB[3] + Z1*W_ocultaB[0][3] + Z2*W_ocultaB[1][3] + Z3*W_ocultaB[2][3] + Z4*W_ocultaB[3][3];

  capa3_out[0] = max(0.0f, U1);
  capa3_out[1] = max(0.0f, U2);
  capa3_out[2] = max(0.0f, U3);
  capa3_out[3] = max(0.0f, U4);

  float V1 = capa3_out[0];
  float V2 = capa3_out[1];
  float V3 = capa3_out[2];
  float V4 = capa3_out[3];

  // ------------------------
  // SALIDA (2 neuronas, SIGMOID)
  // ------------------------
  float O1, O2;
  O1 = b_salida[0] + V1*W_salida[0][0] + V2*W_salida[1][0] + V3*W_salida[2][0] + V4*W_salida[3][0];
  O2 = b_salida[1] + V1*W_salida[0][1] + V2*W_salida[1][1] + V3*W_salida[2][1] + V4*W_salida[3][1];

  // CORRECCIÓN: Usar Sigmoid real
  final_out[0] = sigmoid(O1);
  final_out[1] = sigmoid(O2);
}
// ========================
// Variables
// ========================
bool flag_on = 0;
bool perroBloqueado = false;   // evita repetir PERRO
bool giroBloqueado = false;    // evita repetir GIRO

bool stopActivo = false;
String labelAnterior = "";

// ========================
// Setup
// ========================
void setup() {
  pinMode(Pmalo, INPUT);
  pinMode(Boton, INPUT);
  pinMode(LED_R, OUTPUT);
  qtr.setEmitterPin(LED_V);

  Serial.begin(115200);
  camSerial.begin(9600);

  MotoresBegin();
  // Espera botón
  digitalWrite(LED_R, LOW);
  while (digitalRead(Boton) == HIGH) {}

  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_R, HIGH);
    delay(300);
    digitalWrite(LED_R, LOW);
    delay(300);
  }
  flag_on = 1;
}

// ========================
// Loop
// ========================
void loop() {
   // Detectar si se presiono el boton
  if (digitalRead(Boton) == 0) {
    flag_on = !flag_on;
    // Cambia entre encendido y apagado
    delay(500);
  }
  digitalWrite(LED_R, !flag_on);
  if (flag_on == 1) {  // Si esta encendido
    // Lectura del sensor y parametros para definir el control
    qtr.read();
    uint8_t sens = qtr.getBinary();

    float entradas[8];

    for (int i = 0; i < 8; i++) {
    entradas[i] = bitRead(sens, i);
    // 0.0 o 1.0
    }

    forward(entradas);

    int pwm_base = 115;       // mínimo útil para que el motor sí se mueva
    float zona_muerta = 0.10; // salidas menores a 10% se ignoran

    int motorIzq = compensarPWM(final_out[0], pwm_base, zona_muerta);
    int motorDer = compensarPWM(final_out[1], pwm_base, zona_muerta);

    // Leer label de la cámara
    if (camSerial.available()) {
      String label = camSerial.readStringUntil('\n');
      label.trim();

      if (label == "Pare") {
        stopActivo = true;
        labelAnterior = label;
        Serial.println("Label: Pare | ACCION: Motores OFF - Detenido");
        LeftMotorStop();
        RightMotorStop();
        delay(100);
      }
      else if (label == "INGENIERO") {
        stopActivo = true;
        Serial.println("Label: Ingeniero | ACCION: Motores OFF - Detenido");
        LeftMotorStop();
        RightMotorStop();
        delay(200);
      }
      else if (label == "MEDICO") {
        stopActivo = true;
        Serial.println("Label: Medico | ACCION: Motores OFF - Detenido");
        LeftMotorStop();
        RightMotorStop();
        delay(200);
      }
      else if (label == "CIENTIFICO") {
        stopActivo = true;
        Serial.println("Label: Cientifico | ACCION: Motores OFF - Detenido");
        LeftMotorStop();
        RightMotorStop();
        delay(200);
        }
      else if (label == "GRANJERO") {
        stopActivo = true;
        Serial.println("Label: Granjero | ACCION: Motores OFF - Detenido");
        LeftMotorStop();
        RightMotorStop();
        delay(200);
        }
      else if (label == "BOMBERO") {
        stopActivo = true;
        Serial.println("Label: Bombero | ACCION: Motores OFF - Detenido");
        LeftMotorStop();
        RightMotorStop();
        delay(200);
      }
      else {
        stopActivo = false;

        if (label != labelAnterior) {
          if (label == "GIRO") {
            qtr.read();
            if (!giroBloqueado && (sens & 0b00000111))  {
              Serial.println("Label: GIRO + camino detectado | ACCION: Girando izquierda");
              LeftMotorStop();
              RightMotorStop();
              delay(80);

              LeftMotorReverse(150);
              RightMotorForward(150);
              delay(280);
              giroBloqueado = true;
            }
          }
          else {
            giroBloqueado = false;
          }

          if (label == "PERRO") {
            if (!perroBloqueado) {
              Serial.println("Label: PERRO | ACCION: Retrocediendo");
              LeftMotorStop();
              RightMotorStop();
              delay(100);
              
              LeftMotorReverse(150);
              RightMotorReverse(150);
              delay(500);
              perroBloqueado = true;
            }
            LeftMotorStop();
            RightMotorStop();
            delay(400);
            return;
              
            } 
          else {
            perroBloqueado = false;
          }

          
          if (label == "ENTORNO") {
            Serial.println("Label: Entorno | ACCION: Solo deteccion - Motores sin cambio");
          }

          if (label == "entorno_lab") {
            Serial.println("Label: Entorno | ACCION: Solo deteccion - Motores sin cambio");
          }


        labelAnterior = label;
        }
      }
    }

    // Si stop está activo, no mover motores
    if (stopActivo) {
      LeftMotorStop();
      RightMotorStop();
    return;
    }

    if (digitalRead(Pmalo) == HIGH) {
      LeftMotorStop();
      RightMotorStop();
      return;
    }
    motores(motorIzq, motorDer);
    delay(5); 
  }
}

void motores(int duty_izq, int duty_der) {

  if (duty_izq >= 0) LeftMotorForward(duty_izq);
  else LeftMotorReverse(-duty_izq);

  if (duty_der >= 0) RightMotorForward(duty_der);
  else RightMotorReverse(-duty_der);
}

int compensarPWM(float out, int pwm_base, float zona_muerta) {
  // Limitar por seguridad la salida de la red
  out = constrain(out, 0.0, 1.0);

  // Si la salida es muy pequeña, apagar motor
  if (out <= zona_muerta) {
    return 0;
  }

  // Reescalar la parte útil al rango [pwm_base, 255]
  float out_escalada = (out - zona_muerta) / (1.0 - zona_muerta);
  int pwm = pwm_base + out_escalada * (255 - pwm_base);

  return constrain(pwm, 0, 255);
}

