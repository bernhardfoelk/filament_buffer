#include <StepperDriver_A4988.h>
#include <Arduino.h>

// -------------------------------------------------------------
// Constructor
// -------------------------------------------------------------

Stepper::Stepper()
    : liStep(0),
      liStepOld(0),
      lxEntryAction(false),
      lxStart(false),
      lxReset(false),
      lxFinished(false),
      lxReady(false),
      lxError(false),
      lxJogPos(false),
      lxJogNeg(false),
      ldSetPosWay(0),
      ldSetVelocity(1),
      liStepMode(1),
      ldFeedrate(1),
      liNecessaryCycles(0),
      liActCycles(0),
      oxMotorStep(false),
      oxMotorDir(false),
      liPin_Step(-1),
      liPin_Direction(-1)
{
}

// -------------------------------------------------------------
// Public methods
// -------------------------------------------------------------

void Stepper::setup(int iiPin_Step,
                    int iiPin_Direction,
                    int iiStepMode,
                    int iiFeedrate)
{
    lxError = true;

    if ((iiPin_Direction >= 0) && (iiPin_Step >= 0))
    {
        liPin_Step = iiPin_Step;
        liPin_Direction = iiPin_Direction;

        pinMode(liPin_Step, OUTPUT);
        pinMode(liPin_Direction, OUTPUT);

        lxError = false;
    }

    liStepMode = iiStepMode;
    ldFeedrate = iiFeedrate;
}

void Stepper::reset()
{
    lxReady = false;
    lxReset = true;
}

void Stepper::start(bool ixStart)
{
    lxStart = ixStart;
}

bool Stepper::finished() const
{
    return lxFinished;
}

bool Stepper::ready() const
{
    return lxReady;
}

bool Stepper::error() const
{
    return lxError;
}

void Stepper::setPosWay(int iiSetPosWay)
{
    lxError = true;

    if (liStep == 20)
    {
        lxError = false;
        ldSetPosWay = iiSetPosWay;
    }
}

void Stepper::setVelocity(int iiSetVel)
{
    lxError = true;

    if ((liStep == 20) && (iiSetVel > 0))
    {
        lxError = false;
        ldSetVelocity = iiSetVel;
    }
}

void Stepper::jogPos()
{
    if (liStep == 20)
    {
        lxJogPos = true;
        lxJogNeg = false;
    }
}

void Stepper::jogNeg()
{
    if (liStep == 20)
    {
        lxJogPos = false;
        lxJogNeg = true;
    }
}

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

void Stepper::run()
{
    runStateMachine();
    updateErrorState();
    updateTimerAndCycles();
    writeOutputs();
    debugStepChange();
}

// -------------------------------------------------------------
// Private functions
// -------------------------------------------------------------

void Stepper::runStateMachine()
{
    switch (liStep)
    {
    //***************************************
    // Reset step
    //***************************************
    case 0:
        lxReset = false;
        lxStart = false;
        lxFinished = false;
        lxError = false;

        lxJogPos = false;
        lxJogNeg = false;

        oxMotorDir = false;
        oxMotorStep = false;

        ldSetPosWay = 0;
        ldSetVelocity = 0;

        debugln("Stepper - Ready!");

        liStep = 20;
        break;

    //***************************************
    // Main step
    //***************************************
    case 20:
        lxReady = true;

        if (lxJogPos || lxJogNeg)
        {
            debugln("Stepper - Start jogging!");
            liStep = 1000;
        }
        else if (lxStart &&
                 (ldSetPosWay != 0.0f) &&
                 (ldSetVelocity > 0.0f) &&
                 !lxError)
        {
            debugln("Stepper - Start positioning!");

            liActCycles = 0;
            liStep = 100;
        }
        break;

    //***************************************
    // Position stepper
    //***************************************
    case 100:
        liActCycles = liActCycles + 1;

        liStep = 150; // positioning finished
        if (liActCycles < liNecessaryCycles)
        {
            oxMotorDir = false;
            if (ldSetPosWay > 0)
            {
                oxMotorDir = true;
            }

            oxMotorStep = true;
            liStep = 110;
        }
        break;

    case 110:
        TonStep.IN(true);
        TonStep.PT(calcVelToTime(ldSetVelocity));

        if (TonStep.Q())
        {
            oxMotorStep = false;
            liStep = 120;
        }
        break;

    case 120:
        TonStep.IN(true);
        TonStep.PT(calcVelToTime(ldSetVelocity));

        if (TonStep.Q())
        {
            liStep = 100;
        }
        break;

    //***************************************
    // Position stepper - finished
    //***************************************
    case 150:
        lxFinished = true;

        if (!lxStart)
        {
            debugln("Stepper - Position finished!");

            lxReset = false;
            lxStart = false;
            lxFinished = false;
            lxError = false;

            liStep = 20;
        }
        break;

    //***************************************
    // Jog stepper
    //***************************************
    case 1000:
        oxMotorDir = lxJogNeg && !lxJogPos;
        oxMotorStep = true;

        liStep = 1010;
        break;

    case 1010:
        TonStep.IN(true);
        TonStep.PT(calcVelToTime(50));

        if (TonStep.Q())
        {
            oxMotorStep = false;
            liStep = 1020;
        }
        break;

    case 1020:
        TonStep.IN(true);
        TonStep.PT(calcVelToTime(50));

        if (TonStep.Q())
        {
            lxJogPos = false;
            lxJogNeg = false;

            liStep = 20;
        }
        break;

    default:
        // Fallback
        liStep = 0;
        break;
    }
}

void Stepper::updateErrorState()
{
    // Error: ungültiger Step-Mode
    if ((liStepMode != 1) &&
        (liStepMode != 2) &&
        (liStepMode != 4) &&
        (liStepMode != 8) &&
        (liStepMode != 16))
    {
        lxError = true;
    }
}

void Stepper::updateTimerAndCycles()
{
    // Timer call
    TonStep.run();
    TonStep.IN(false);

    // calculate feedrate and stepmode to number of cycles (fullstep..200 steps is one turn)
    if (!lxError && ldFeedrate > 0.0)
    {
        ldTemp = 200.0 * static_cast<double>(liStepMode);
        ldTemp = ldTemp / ldFeedrate; // ldFeedrate = mm/U
        ldTemp = ldTemp * abs(ldSetPosWay);
        liNecessaryCycles = static_cast<unsigned long>(ldTemp);
    }
    else
    {
        liNecessaryCycles = 0;
    }
}

void Stepper::writeOutputs()
{
    digitalWrite(liPin_Step, oxMotorStep);
    digitalWrite(liPin_Direction, oxMotorDir);
}

void Stepper::debugStepChange()
{
    if (liStep != liStepOld)
    {
        liStepOld = liStep;
        lxEntryAction = true;

        debug("Stepper - Aktueller Schritt: ");
        debugln(liStep);
    }
}

unsigned long Stepper::calcVelToTime(double idVelocity)
{
    if (idVelocity <= 0.0 || ldFeedrate <= 0.0)
        return 0;

    const double ldStepsPerRev = 200.0 * static_cast<double>(liStepMode);
    const double ldStepsPerMillimeter = ldStepsPerRev / ldFeedrate; // mm/U
    const double ldStepsPerSec = idVelocity * ldStepsPerMillimeter;

    if (ldStepsPerSec <= 0.0)
        return 0;

    const double ldHalfPeriodMicroseconds =
        1000000.0 / ldStepsPerSec / 2.0;

    return static_cast<unsigned long>(ldHalfPeriodMicroseconds);
}

/*#include <StepperDriver_A4988.h>

Stepper::Stepper()
    : liStep(0),
      liStepOld(0),
      lxEntryAction(false),
      lxStart(false), lxReset(false), lxFinished(false), lxReady(false), lxError(false),
      lxJogPos(false), lxJogNeg(false),
      ldSetPosWay(0), ldSetVelocity(1),
      liStepMode(1), ldFeedrate(1),
      liNecessaryCycles(0), liActCycles(0),
      oxMotorStep(false), oxMotorDir(false),
      liPin_Step(-1), liPin_Direction(-1) // oder 0; -1 = ungültig
{
}

void Stepper::setup(int iiPin_Step, int iiPin_Direction, int iiStepMode, int iiFeedrate)
{

    lxError = true;
    if ((iiPin_Direction >= 0) && (iiPin_Step >= 0)){
        liPin_Step = iiPin_Step;
        liPin_Direction = iiPin_Direction;

        pinMode(liPin_Step, OUTPUT);
        pinMode(liPin_Direction, OUTPUT);

        lxError = false;
    }

    liStepMode = iiStepMode;
    ldFeedrate = iiFeedrate;
}

void Stepper::reset()
{
    lxReady = false;
    lxReset = true;
}

void Stepper::start(bool ixStart)
{
    lxStart = ixStart;
}

bool Stepper::finished() const
{
    return lxFinished;
}

bool Stepper::ready() const
{
    return lxReady;
}

bool Stepper::error() const
{
    return lxError;
}

void Stepper::setPosWay(int iiSetPosWay)
{
    lxError = true;
    if (liStep == 20)
    {
        lxError = false;
        ldSetPosWay = iiSetPosWay;
    }
}

void Stepper::setVelocity(int iiSetVel)
{
    lxError = true;
    if ((liStep == 20) && (iiSetVel > 0))
    {
        lxError = false;
        ldSetVelocity = iiSetVel;
    }
}

void Stepper::jogPos()
{
    if (liStep == 20)
    {
        lxJogPos = true;
        lxJogNeg = false;
    }
}

void Stepper::jogNeg()
{
    if (liStep == 20)
    {
        lxJogPos = false;
        lxJogNeg = true;
    }
}

unsigned long Stepper::calcVelToTime(double idVelocity)
{
    if (idVelocity <= 0.0 || ldFeedrate <= 0.0) return 0;

    const double ldStepsPerRev = 200.0 * static_cast<double>(liStepMode);
    const double ldStepsPerMillimeter  = ldStepsPerRev / ldFeedrate; // mm/U
    const double ldStepsPerSec = idVelocity * ldStepsPerMillimeter;

    if (ldStepsPerSec <= 0.0) return 0;

    const double ldHalfPeriodMicroseconds = 1000000.0 / ldStepsPerSec / 2.0;
    return static_cast<unsigned long>(ldHalfPeriodMicroseconds);
}

void Stepper::run()
{

    switch (liStep)
    {

    //***************************************
    // Reset step
    //***************************************
    case 0:

        lxReset = false;
        lxStart = false;
        lxFinished = false;
        lxError = false;

        lxJogPos = false;
        lxJogNeg = false;

        oxMotorDir = false;
        oxMotorStep = false;

        ldSetPosWay = 0;
        ldSetVelocity = 0;

        Serial.println("Stepper - Ready!");

        liStep = 20;
        break;

    //***************************************
    // Main step
    //***************************************
    case 20:

        lxReady = true;

        if (lxJogPos || lxJogNeg)
        {

            Serial.println("Stepper - Start jogging!");

            liStep = 1000;
        }
        else if (lxStart && (ldSetPosWay != 0.0f) && (ldSetVelocity > 0.0f) && !lxError)
        {

            Serial.println("Stepper - Start positioning!");

            liActCycles = 0;

            liStep = 100;
        }
        break;

    //***************************************
    // Position stepper
    //***************************************
    case 100:

        liActCycles = liActCycles + 1;

        liStep = 150; // positioning finished
        if (liActCycles < liNecessaryCycles)
        {

            oxMotorDir = false;
            if (ldSetPosWay > 0)
            {
                oxMotorDir = true;
            }
            oxMotorStep = true;

            liStep = 100;
        }
        break;

    case 110:

        TonStep.IN(true);
        TonStep.PT(calcVelToTime(ldSetVelocity));

        if (TonStep.Q())
        {

            oxMotorStep = false;

            liStep = 120;
        }
        break;

    case 120:

        TonStep.IN(true);
        TonStep.PT(calcVelToTime(ldSetVelocity));

        if (TonStep.Q())
        {

            liStep = 100;
        }
        break;

    //***************************************
    // Position stepper - finished
    case 150:

        lxFinished = true;

        if (!lxStart)
        {
            Serial.println("Stepper - Position finished!");

            lxReset = false;
            lxStart = false;
            lxFinished = false;
            lxError = false;

            liStep = 20;
        }
        break;


    //***************************************
    // Jog stepper
    //***************************************
    case 1000:

        oxMotorDir = lxJogNeg && !lxJogPos;
        oxMotorStep = true;

        liStep = 1010;
        break;

    case 1010:
        TonStep.IN(true);
        TonStep.PT(calcVelToTime(50));

        if (TonStep.Q())
        {
            oxMotorStep = false;

            liStep = 1020;
        }
        break;

    case 1020:
        TonStep.IN(true);
        TonStep.PT(calcVelToTime(50));

        if (TonStep.Q())
        {
            lxJogPos = false;
            lxJogNeg = false;

            liStep = 20;
        }
        break;
    }

    // error
    if ((liStepMode != 1) && (liStepMode != 2) && (liStepMode != 4) && (liStepMode != 8) && (liStepMode != 16))
    {
        lxError = true;
    }

    // Timer call
    TonStep.run();
    TonStep.IN(false);

    // calculate feedrate and stepmode to number of cycles (fullstep..200 steps is one turn)
    if (!lxError && ldFeedrate > 0.0) {
        ldTemp = 200.0 * static_cast<double>(liStepMode);
        ldTemp  = ldTemp / ldFeedrate; // ldFeedrate = mm/U
        ldTemp   = ldTemp * abs(ldSetPosWay);
        liNecessaryCycles = static_cast<unsigned long>(ldTemp);
    } else {
        liNecessaryCycles = 0;
    }

    // write outputs
    digitalWrite(liPin_Step, oxMotorStep);
    digitalWrite(liPin_Direction, oxMotorDir);

    // Debug: log change of step variable
    if (liStep != liStepOld)
    {
        liStepOld = liStep;
        lxEntryAction = true;
        Serial.println("Aktueller Schritt: ");
        Serial.println(liStep);
    }
}
*/