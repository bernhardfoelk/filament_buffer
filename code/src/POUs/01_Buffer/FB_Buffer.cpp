//
// Created by bernhard on 11.09.26.
//

#include "../01_Buffer/FB_Buffer.h"
#include "Arduino.h"

FB_Buffer::FB_Buffer()
    : liStep(0),
      liNStep(0),
      liStepOld(0),
      lxEntryAction(false),
      lxTemp(false),
      liTemp(0),
      liMicrostepping(8), // values for 1/1 step configuration
      lrSpeed(1250 * liMicrostepping),
      lrReferenceSpeed(325 * liMicrostepping),
      lrAcceleration(12500 * liMicrostepping),
      motor(PIN_MOTOR_STEP,
            PIN_MOTOR_DIR,
            200 * liMicrostepping) {
    // set pinmode for input variables
    pinMode(PIN_BUTTON_FORWARD, INPUT_PULLUP);
    pinMode(PIN_BUTTON_BACKWARD, INPUT_PULLUP);
    pinMode(PIN_ENDSTOP_POS, INPUT_PULLUP);
    pinMode(PIN_ENDSTOP_NEG, INPUT_PULLUP);
}

void FB_Buffer::run() {
    switch (liStep) {
        //***************************************
        // Reset
        //***************************************
        case 0:
            if (lxEntryAction) {
                lxEntryAction = false;
            }

            if (true) {
                motor.reset();
                debugln("FB_Buffer - Motor reset.");
                liStep = 4;
            }
            break;

        case 4:
            if (lxEntryAction) {
                lxEntryAction = false;
            }

            if (motor.isReady()) {
                debugln("FB_Buffer - Motor bereit.");

                liStep = 20;
            }
            break;


        //***************************************
        // Main step
        //***************************************
        case 20:
            if (lxEntryAction) {
                lxEntryAction = false;
            }

            if (ixButtonForward) {
                liStep = 2000;
            } else if (ixButtonBackward) {
                liStep = 3000;
            } else if (ixEndstopNeg) {
                liStep = 1000; // Position stepper
            }
            break;


        //***************************************
        // Position stepper
        //***************************************
        case 1000:
            if (lxEntryAction) {
                // Motors for second
                motor.setSpeed(lrSpeed);
                motor.setAcceleration(lrAcceleration);
                motor.setTargetPosition(25); // 25mm

                motor.setStart();

                lxEntryAction = false;
            }

            if (motor.isFinished()) {
                liNStep = 20;
                liStep = 9000;
            } else if (ixEndstopPos) {
                liStep = 0; // instant stop
            }
            break;


        //***************************************
        // Jog stepper in positive direction
        //***************************************
        case 2000:
            if (lxEntryAction) {
                motor.setSpeed(lrSpeed);
                motor.setAcceleration(lrAcceleration);
                motor.setTargetPosition(500); // 500mm

                motor.setStart();

                lxEntryAction = false;
            }

            if (motor.isFinished()) {
                liNStep = 0;
                liStep = 9000;
            } else if (!ixButtonForward) {
                liStep = 0;
            }
            break;


        //***************************************
        // Jog stepper in negative direction
        //***************************************
        case 3000:
            if (lxEntryAction) {
                motor.setSpeed(lrSpeed);
                motor.setAcceleration(lrAcceleration);
                motor.setTargetPosition(-500); // 500mm

                motor.setStart();

                lxEntryAction = false;
            }

            if (motor.isFinished()) {
                liNStep = 0;
                liStep = 9000;
            } else if (!ixButtonBackward) {
                liStep = 0;
            }
            break;


        //***************************************
        // Check if both buttons are released
        // and set current position to zero
        //***************************************
        case 9000:
            if (lxEntryAction) {
                lxEntryAction = false;
            }

            if (!ixButtonForward && !ixButtonBackward && !ixEndstopNeg) {
                liStep = 9010;
            }
            break;

        case 9010:
            if (lxEntryAction) {
                motor.setReference(0);

                lxEntryAction = false;
            }

            if (motor.isFinished()) {
                liStep = liNStep;
            }
            break;
    }

    motor
            .
            run();

    readInputs();

    updateEdges();

    updateTimers();

    debugStepChange();
}

void FB_Buffer::readInputs() {
    TonDebounceButtonForward.IN(!(digitalRead(PIN_BUTTON_FORWARD)));
    TonDebounceButtonForward.PT(10);
    TonDebounceButtonForward.run();
    ixButtonForward = TonDebounceButtonForward.Q();

    TonDebounceButtonBackward.IN(!(digitalRead(PIN_BUTTON_BACKWARD)));
    TonDebounceButtonBackward.PT(10);
    TonDebounceButtonBackward.run();
    ixButtonBackward = TonDebounceButtonBackward.Q();

    TonDebounceEndstopPos.IN(!(digitalRead(PIN_ENDSTOP_POS)));
    TonDebounceEndstopPos.PT(10);
    TonDebounceEndstopPos.run();
    ixEndstopPos = TonDebounceEndstopPos.Q();

    TonDebounceEndstopNeg.IN(!(digitalRead(PIN_ENDSTOP_NEG)));
    TonDebounceEndstopNeg.PT(10);
    TonDebounceEndstopNeg.run();
    ixEndstopNeg = TonDebounceEndstopNeg.Q();
}

void FB_Buffer::updateEdges() {
    EdgeButtonForward.run(ixButtonForward);
    EdgeButtonBackward.run(ixButtonBackward);
    EdgeEndstopPos.run(ixEndstopPos);
    EdgeEndstopNeg.run(ixEndstopNeg);
    if (EdgeButtonForward.EdgeNeg()) {
        debugln("FB_Buffer - Button Forward: Negative Flanke!");
    }
    if (EdgeButtonBackward.EdgeNeg()) {
        debugln("FB_Buffer - Button Backward: Negative Flanke!");
    }
    if (EdgeEndstopPos.EdgeNeg()) {
        debugln("FB_Buffer - Button Endstop pos.: Negative Flanke!");
    }
    if (EdgeEndstopNeg.EdgeNeg()) {
        debugln("FB_Buffer - Button Endstop neg.: Negative Flanke!");
    }
}

void FB_Buffer::updateTimers() {
    // Timer call
    if (liStep != liStepOld) {
        TonStep.IN(false);
        TonStepF.IN(false);
    }

    TonStep.run();
    TonStep.IN(false);

    TonStepF.run();
    TonStepF.IN(false);
}

void FB_Buffer::debugStepChange() {
    if (liStep != liStepOld) {
        liStepOld = liStep;
        lxEntryAction = true;
        debug("FB_Buffer - Aktueller Schritt: ");
        debugln(liStep);
    }
}
