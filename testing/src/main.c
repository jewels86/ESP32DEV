#include "hardware/abspins.h"

void app_main() {
    dPinIN(13);
    dPinOUT(12);
    while (true) {
        int value = dRead(13);
        if (value == HIGH) dWrite(12, HIGH);
        else dWrite(12, LOW);
    }
    
}