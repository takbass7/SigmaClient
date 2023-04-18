#ifndef speed
#define speed 115200


#include <Arduino.h>

class Sigma {

public:
    Sigma();
	Sigma(unsigned long baudrate);

    void print(String str);
    void println(String str);
    void readln();
    void clearread();

    bool readComplete = false;
    String inputString = "";
};

#endif