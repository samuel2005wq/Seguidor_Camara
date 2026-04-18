#ifndef QTR8A_H
#define QTR8A_H

#include <Arduino.h>

/* =========================================================
   Enum de dirección
   ========================================================= */
enum QTR_Direction {
  QTR_IZQUIERDA,
  QTR_DERECHA,
  QTR_CENTRO
};

/* =========================================================
   Clase QTR8A
   ========================================================= */
class QTR8A {
public:
  /* ---------------------------
       Constructor
       --------------------------- */
  QTR8A(
    const uint8_t *sens_pins,
    uint16_t threshold = 600) {
    _threshold = threshold;
    _emitterEnabled = false;
    _last_position = 0;

    for (uint8_t i = 0; i < 8; i++) {
      _sens[i] = sens_pins[i];
      pinMode(_sens[i], INPUT);
    }
  }

  /* ---------------------------
       Configurar pin del emisor
       --------------------------- */
  void setEmitterPin(uint8_t pin) {
    _emitterPin = pin;
    pinMode(_emitterPin, OUTPUT);
    digitalWrite(_emitterPin, LOW);
    _emitterEnabled = true;
  }

  void emitterOn() {
    if (_emitterEnabled)
      digitalWrite(_emitterPin, HIGH);
  }

  void emitterOff() {
    if (_emitterEnabled)
      digitalWrite(_emitterPin, LOW);
  }

  /* ---------------------------
       Lectura principal
       --------------------------- */
  void read() {
    int32_t sum = 0;
    _sens_on = 0;
    _sens_bin = 0;

    if (_emitterEnabled)
      digitalWrite(_emitterPin, HIGH);

    for (uint8_t i = 0; i < 8; i++) {
      analogRead(_sens[i]);  // selecciona canal
      delayMicroseconds(20);  // tiempo de asentamiento (clave)
      uint16_t val = analogRead(_sens[i]);

      if (val > _threshold) {
        _sens_on++;
        _sens_bin |= (1 << i);
        sum += _peso[i];
      }
    }

    if (_emitterEnabled)
      digitalWrite(_emitterPin, LOW);

    if (_sens_on > 0) {
      _position = sum / _sens_on;
      _last_position = _position;  // guardar última posición válida
    } else {
      // Línea perdida: usar último lado conocido
      if (_last_position < 0)
        _position = _lost_left;  // girar fuerte a la izquierda
      else
        _position = _lost_right;  // girar fuerte a la derecha
    }

    _calcDirection();
  }

  /* ---------------------------
       Getters
       --------------------------- */
  int16_t getPosition() const {
    return _position;
  }
  uint8_t getBinary() const {
    return _sens_bin;
  }
  uint8_t getSensorsOn() const {
    return _sens_on;
  }
  QTR_Direction getDirection() const {
    return _direction;
  }

private:
  /* ---------------------------
       Pines y parámetros
       --------------------------- */
  uint8_t _sens[8];
  uint16_t _threshold;
  uint8_t _emitterPin;
  bool _emitterEnabled;

  /* ---------------------------
       Variables internas
       --------------------------- */
  int16_t _position;
  uint8_t _sens_on;
  uint8_t _sens_bin;
  QTR_Direction _direction;
  int16_t _last_position;
  const int16_t _lost_left = -40;
  const int16_t _lost_right = 40;


  /* ---------------------------
       Pesos centrados en 0
       --------------------------- */
  const int8_t _peso[8] = { -35, -25, -15, -5, 5, 15, 25, 35 };

  /* ---------------------------
       Dirección (binaria)
       --------------------------- */
  void _calcDirection() {
    if ((_sens_bin & 0b00000111) == 0b00000111)
      _direction = QTR_IZQUIERDA;
    else if ((_sens_bin & 0b11100000) == 0b11100000)
      _direction = QTR_DERECHA;
    else
      _direction = QTR_CENTRO;
  }
};

#endif