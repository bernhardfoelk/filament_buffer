//
// Created by bernhard on 10.09.26.
//

#include "StepperDriverAbsolut.h"

#include <math.h>

StepperDriverAbsolut::StepperDriverAbsolut(uint8_t iiStepPin, uint8_t iiDirPin,
                                           long ilStepsPerRotation)
    : fbStepper(AccelStepper::DRIVER, iiStepPin, iiDirPin),
      llOffsetToReference(0),
      lfStepsPerRotation(ilStepsPerRotation),
      lfStepsPerMm(281.0f),
      lrTargetPosition(0.0f),
      lrSpeed(400.0f),
      lrReferenceSpeed(150.0f),
      lxStart(false),
      lxStartReference(false),
      lxFinished(true),
      lxReferenced(false),
      liStep(0) {
    fbStepper.setMaxSpeed(lrSpeed);
    fbStepper.setAcceleration(800.0f);
}

// ---------------- Eingänge ----------------

void StepperDriverAbsolut::setTargetPosition(float irValue) {
    lrTargetPosition = irValue;
}

void StepperDriverAbsolut::setStart() {
    lxStart = true;

    lxFinished = false;
    lxReferenced = false;
}

void StepperDriverAbsolut::setStartReference() {
    lxStartReference = true;

    lxFinished = false;
    lxReferenced = false;
}

// ---------------- Parametrierung ----------------

void StepperDriverAbsolut::setSpeed(float irValue) {
    lrSpeed = irValue;
}

void StepperDriverAbsolut::setAcceleration(float irValue) {
    fbStepper.setAcceleration(irValue);
}

void StepperDriverAbsolut::setSpeedReference(float irValue) {
    lrReferenceSpeed = irValue;
}

void StepperDriverAbsolut::setOffsetToReference(long ilValue) {
    llOffsetToReference = ilValue;
}

void StepperDriverAbsolut::setReference(long ilValue) {
    lxSetReference = true;

    lxFinished = false;
    lxReferenced = false;

    llReferencePosition = ilValue;
}

// ---------------- zyklischer Aufruf ----------------

void StepperDriverAbsolut::run() {
    EdgeStart.run(lxStart);
    EdgeStartReference.run(lxStartReference);
    SetReference.run(lxSetReference);

    switch (liStep) {
        case 0:
            fbStepper.setCurrentPosition(fbStepper.currentPosition());

            lxStart = false;
            lxStartReference = false;

            lxReady = false;

            liStep = 20;
            break;


        case 20:
            lxReady = true;

            if (lxStart) {
                llTargetStep = MmToSteps(lrTargetPosition);

                lxStart = false;

                liStep = 100;
            } else if (lxSetReference) {
                lxSetReference = false;

                liStep = 200;
            }
            break;

        // Position motor
        case 100:
            fbStepper.setMaxSpeed(lrSpeed);
            fbStepper.moveTo(-llTargetStep);
            fbStepper.run();

            if (fbStepper.distanceToGo() == 0) {
                lxFinished = true;
                liStep = 0;
            }
            break;

        // Set Position
        case 200:

            fbStepper.setCurrentPosition(llReferencePosition);

            lxFinished = true;

            liStep = 0;
            break;
    }
}

void StepperDriverAbsolut::reset() {
    liStep = 0;
}

// ---------------- Ausgänge ----------------

bool StepperDriverAbsolut::isFinished() const {
    return lxFinished;
}

bool StepperDriverAbsolut::isReady() const {
    return lxReady;
}

bool StepperDriverAbsolut::isReferenced() const {
    return lxReferenced;
}

// ---------------- intern ----------------
long StepperDriverAbsolut::MmToSteps(float irMm) {
    return (long) lround(irMm * lfStepsPerMm);
}
