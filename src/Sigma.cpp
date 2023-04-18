
#include "Sigma.h"

Sigma::Sigma(){
    Serial.begin(speed );
}

Sigma::Sigma(unsigned long baudrate) {
    Serial.begin(baudrate );
}

void Sigma::print(String str) {
    Serial.print(str);
}

void Sigma::println(String str) {
    Serial.println(str);
}

void Sigma::readln() {
    while (Serial.available()) {
        char inChar = (char)Serial.read();

        this->inputString += inChar;
        Serial.print(inChar);

        if (inChar == '\n') {
          this->readComplete = true;
        }
    }
}

void Sigma::clearread() {
    this->inputString = "";
    this->readComplete = false;
}