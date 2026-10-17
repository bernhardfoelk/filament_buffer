//
// Created by bernhard on 11.09.26.
//

#ifndef FB_MOTORS_H
#define FB_MOTORS_H

#include "Ton.h"
#include "EdgePosNeg.h"
#include "app_config.h"
#include "StepperDriverAbsolut.h"

class FB_Buffer {
public:
    FB_Buffer();

    void run();

private:
    // inputs
    bool ixButtonForward;
    bool ixButtonBackward;
    bool ixEndstopPos;
    bool ixEndstopNeg;

    // outputs

    // local
    int liStep;
    int liNStep;
    int liStepOld;
    bool lxEntryAction;
    bool lxTemp;
    int liTemp;

    int liMicrostepping;
    float lrSpeed;
    float lrReferenceSpeed;
    float lrAcceleration;

    Ton TonStep;
    Ton TonStepF;

    StepperDriverAbsolut motor;

    Ton TonDebounceButtonForward;
    Ton TonDebounceButtonBackward;
    Ton TonDebounceEndstopPos;
    Ton TonDebounceEndstopNeg;

    EdgePosNeg EdgeButtonForward;
    EdgePosNeg EdgeButtonBackward;
    EdgePosNeg EdgeEndstopPos;
    EdgePosNeg EdgeEndstopNeg;

    void readInputs();

    void updateEdges();

    void updateTimers();

    void debugStepChange();
};


#endif //FB_MOTORS_H
