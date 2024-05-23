#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
  private:
    int pin;
    bool state;
    bool lastState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;

  public:
    Button(int pinNumber);
    bool isPressed();
};

#endif
