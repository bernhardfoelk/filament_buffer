
#ifndef STEPPERDRIVER_A4988_H
#define STEPPERDRIVER_A4988_H

#pragma once

#include "TonMicros.h"

#define DEBUGGING true

#if DEBUGGING == true
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

class Stepper
{
public:
    Stepper();

    void setup(int iiPin_Step,
               int iiPin_Direction,
               int iiStepMode,
               int iiFeedrate);

    void reset();
    void start(bool ixStart);

    bool finished() const;
    bool ready() const;
    bool error() const;

    void setPosWay(int iiSetPosWay);
    void setVelocity(int iiSetVel);

    void jogPos();
    void jogNeg();

    void run();

private:
    // Hauptlogik wie in main.cpp aufgeteilt
    void runStateMachine();
    void updateErrorState();
    void updateTimerAndCycles();
    void writeOutputs();
    void debugStepChange();
    unsigned long calcVelToTime(double idVelocity);

    // Zustandsvariablen
    int liStep;
    int liStepOld;
    bool lxEntryAction;

    bool lxStart;
    bool lxReset;
    bool lxFinished;
    bool lxReady;
    bool lxError;

    bool lxJogPos;
    bool lxJogNeg;

    double ldSetPosWay;
    double ldSetVelocity;

    int liStepMode;
    double ldFeedrate;

    unsigned long liNecessaryCycles;
    unsigned long liActCycles;

    bool oxMotorStep;
    bool oxMotorDir;

    int liPin_Step;
    int liPin_Direction;

    // Hilfsgröße für Berechnungen
    double ldTemp;

    // Timer für Schrittzeiten
    TonMicros TonStep;
};

#endif

/*
#ifndef STEPPERDRIVER_A4988_H
#define STEPPERDRIVER_A4988_H

#include <Arduino.h>
#include <EdgePosNeg.h>
#include <TonMicros.h>

class Stepper
{
public:
    Stepper(); // Standardkonstruktor
    void setup(int iiPin_Step, int iiPin_Direction, int iiStepMode, int iiFeedrate);
    void run(); // muss regelmäßig aufgerufen werden
    void reset();
    void start(bool ixStart);
    bool finished() const; // Rückgabe ob Positionierung fertig ist
    bool ready() const;
    bool error() const;
    void setPosWay(int iiSetPosWay);
    void setVelocity(int iiSetVel);
    void jogPos();
    void jogNeg();

private:
    // General
    int liStep = 0;
    int liStepOld = 0;
    bool lxEntryAction = false;
    double ldTemp;

    // configuration
    int liPin_Direction;
    int liPin_Step;

    // parameter
    double ldSetPosWay;
    double ldSetVelocity;
    int liStepMode;
    double ldFeedrate;

    // status/controll flags
    bool lxStart;
    bool lxReset;
    bool lxFinished;
    bool lxReady;
    bool lxError;
    bool lxJogPos;
    bool lxJogNeg;

    // calculation/runtime
    unsigned long liNecessaryCycles;
    unsigned long liActCycles;
    //unsigned long liStepDelayTime_ms;

    // IO pins
    bool oxMotorStep;
    bool oxMotorDir;

    // helper
    TonMicros TonStep;
    EdgePosNeg Edge;
    unsigned long calcVelToTime(double idVelocity);
};

#endif
*/