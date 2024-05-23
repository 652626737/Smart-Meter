#include "Button.h"

Button::Button(int pinNumber) {
    pin = pinNumber;
    state = LOW;
    lastState = LOW;
    lastDebounceTime = 0;
    debounceDelay = 50;
    pinMode(pin, INPUT);
}

bool Button::isPressed() {
    int reading = digitalRead(pin);
    
    if (reading != lastState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != state) {
            state = reading;
        }
    }

    lastState = reading;
    return state == HIGH;
}
