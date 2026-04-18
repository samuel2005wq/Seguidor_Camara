#ifndef MOTORES_H
#define MOTORES_H

#include <Arduino.h>

void setupPWM_fast() {

  // ---------- TIMER1 (Motor B) ----------
  // OC1B → pin 10
  TCCR1A = 0;
  TCCR1B = 0;

  // Fast PWM 8-bit, no invertido en OC1B
  // WGM13:0 = 0b0101
  TCCR1A = (1 << WGM10) | (1 << COM1B1);
  TCCR1B = (1 << WGM12) | (1 << CS10);   // prescaler = 1

  OCR1B = 0;
  pinMode(PWMB, OUTPUT);


  // ---------- TIMER2 (Motor A) ----------
  // OC2A → pin 11
  TCCR2A = 0;
  TCCR2B = 0;

  // Fast PWM 8-bit, no invertido en OC2A
  // WGM22:0 = 0b011
  TCCR2A = (1 << WGM20) | (1 << WGM21) | (1 << COM2A1);
  TCCR2B = (1 << CS20);                  // prescaler = 1

  OCR2A = 0;
  pinMode(PWMA, OUTPUT);
}


// ========================
// PWM (0–255)
// ========================
void pwmB(uint8_t duty) {
  duty = constrain(duty, 0, 255);
  OCR1B = duty;
}
void pwmA(uint8_t duty) {
  duty = constrain(duty, 0, 255);
  OCR2A = duty;
  }

// ========================
// MOTOR A
// ========================
void LeftMotorForward(uint8_t duty) {
  duty = constrain(duty, 0, 255);
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  pwmA(duty);
}

void LeftMotorReverse(uint8_t duty) {
  duty = constrain(duty, 0, 255);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  pwmA(duty);
}

void LeftMotorStop() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, HIGH);
  pwmA(0);
}

// ========================
// MOTOR B
// ========================
void RightMotorForward(uint8_t duty) {
  duty = constrain(duty, 0, 255);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  pwmB(duty);
}

void RightMotorReverse(uint8_t duty) {
  duty = constrain(duty, 0, 255);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  pwmB(duty);
}

void RightMotorStop() {
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, HIGH);
  pwmB(0);
}

// Función para inicializar los motores
void MotoresBegin() {
  // Pines de control del driver
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  // Apagar driver mientras se configura
  digitalWrite(STBY, LOW);

  // Configurar PWM
  setupPWM_fast();

  // Activar driver
  digitalWrite(STBY, HIGH);

  // Motores detenidos al iniciar
  LeftMotorStop();
  RightMotorStop();
}

#endif