#include "app_config.h"
#include "POUs/01_Buffer//FB_Buffer.h"

// -------------------------------------------------------------
// Global variables (only used in this file)
// -------------------------------------------------------------

// General --> Definition der FB's/Klassen
static FB_Buffer fbBuffer;

// Interface variables

// -------------------------------------------------------------
// Arduino setup / loop
// -------------------------------------------------------------

void setup() {
    // Start serial interface
    Serial.begin(9600);
    delay(500);

    debugln("Debugging started..");
}

void loop() {
    fbBuffer.run();
}
