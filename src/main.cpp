#include "app_config.h"

// -------------------------------------------------------------
// Global variables (only used in this file)
// -------------------------------------------------------------

// General
int liStep = 0;
int liStepOld = 0;
bool lxEntryAction = false;
bool lxTemp = false;
bool lxStartFeeding = false;
bool lxJogPos = false;
bool lxJogNeg = false;

// Inputs
bool ixFilamentPresent = false;
bool ixEndposPos = false;
bool ixEndposNeg = false;
bool ixFeedForw = false;
bool ixFeedBackw = false;

// Stepper motor
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *extruder = NULL; // Pointer auf den Motor

// Timer
Ton TonStep;
Ton TonStepF;
Ton TonStartFeeding;
Ton TonFeedForw;
Ton TonFeedBackw;

// Edge detection for buttons
EdgePosNeg Edge_EndstopPos;
EdgePosNeg Edge_EndstopNeg;
EdgePosNeg Edge_FeedForw;
EdgePosNeg Edge_FeedBackw;

Ton TonDebounceFeedForw;
Ton TonDebounceFeedBackw;
Ton TonDebounceEndstopPos;
Ton TonDebounceEndstopNeg;

// Functions
void readInputs();

void updateEdges();

void updateTimersAndJog();

void runStateMachine();

void debugStepChange();

// -------------------------------------------------------------
// Arduino setup / loop
// -------------------------------------------------------------

void setup()
{
  // Define button pins as input
  pinMode(PIN_FEED_FORW_BUTTON, INPUT_PULLUP);
  pinMode(PIN_FEED_BACKW_BUTTON, INPUT_PULLUP);
  pinMode(PIN_ENDSTOP_NEGATIVE, INPUT_PULLUP);
  pinMode(PIN_ENDSTOP_POSITIVE, INPUT_PULLUP);

  /// FastAccelStepper initialisieren
  engine.init();
  extruder = engine.stepperConnectToPin(PIN_STEPPER_STEP);

  if (extruder)
  {
    extruder->setDirectionPin(PIN_STEPPER_DIR);

    // Invertierung falls nötig (entspricht setPinsInverted)
    extruder->setDirectionPin(PIN_STEPPER_DIR, false);
    extruder->setEnablePin(0xFFFF); // Falls kein physischer Enable-Pin genutzt wird

    // Echte Hardware-Werte ohne Rechenfehler oder Überlauf!
    extruder->setSpeedInHz(1000 * 2);
    extruder->setAcceleration(8000 * 2);

    extruder->setCurrentPosition(0);
  }

  // Start serial interface
  Serial.begin(9600);
  delay(2000);
  debugln("Debugging started..");
}

void loop()
{
  readInputs();
  updateEdges();
  runStateMachine();
  updateTimersAndJog();
  debugStepChange();
}

// -------------------------------------------------------------
// Helper functions
// -------------------------------------------------------------

void readInputs()
{
  TonDebounceEndstopPos.IN(digitalRead(PIN_ENDSTOP_POSITIVE));
  TonDebounceEndstopPos.PT(10);
  TonDebounceEndstopPos.run();
  ixEndposPos = TonDebounceEndstopPos.Q();

  TonDebounceEndstopNeg.IN(digitalRead(PIN_ENDSTOP_NEGATIVE));
  TonDebounceEndstopNeg.PT(10);
  TonDebounceEndstopNeg.run();
  ixEndposNeg = TonDebounceEndstopNeg.Q();

  TonDebounceFeedForw.IN(!(digitalRead(PIN_FEED_FORW_BUTTON)));
  TonDebounceFeedForw.PT(10);
  TonDebounceFeedForw.run();
  ixFeedForw = TonDebounceFeedForw.Q();

  TonDebounceFeedBackw.IN(!(digitalRead(PIN_FEED_BACKW_BUTTON)));
  TonDebounceFeedBackw.PT(10);
  TonDebounceFeedBackw.run();
  ixFeedBackw = TonDebounceFeedBackw.Q();
}

void updateEdges()
{
  Edge_EndstopPos.run(ixEndposPos);
  Edge_EndstopNeg.run(ixEndposNeg);
  Edge_FeedForw.run(ixFeedForw);
  Edge_FeedBackw.run(ixFeedBackw);
  if (Edge_EndstopPos.EdgePos())
  {
    debugln("Endstop positiv: Positive Flanke!");
  }
  if (Edge_EndstopNeg.EdgePos())
  {
    debugln("Endstop negativ: Positive Flanke!");
  }
  if (Edge_FeedForw.EdgePos())
  {
    debugln("Sensor feed forward: Positive Flanke!");
  }
  if (Edge_FeedBackw.EdgePos())
  {
    debugln("Sensor feed backward: Positive Flanke!");
  }
}

void updateTimersAndJog()
{
  // Timer call
  TonStep.run();
  // TonStep.IN(false);

  TonStepF.run();
  // TonStepF.IN(false);

  TonStartFeeding.run();
  TonStartFeeding.IN(false);

  // Jog buttons with delay
  TonFeedForw.IN(ixFeedForw);
  TonFeedForw.PT(500);
  TonFeedForw.run();
  lxJogPos = TonFeedForw.Q();

  TonFeedBackw.IN(ixFeedBackw);
  TonFeedBackw.PT(500);
  TonFeedBackw.run();
  lxJogNeg = TonFeedBackw.Q();
}

void runStateMachine()
{
  switch (liStep)
  {
  //***************************************
  // Stop motor
  //***************************************
  case 0:
    if (lxEntryAction)
    {
      extruder->stopMove();
      lxEntryAction = false;
    }

    if (!extruder->isRunning())
    {
      extruder->setCurrentPosition(0);
      liStep = 20;
    }
    break;

  //***************************************
  // Main step
  //***************************************
  case 20:
    if (lxEntryAction)
    {
      lxEntryAction = false;
    }

    TonStartFeeding.IN(ixEndposNeg);
    TonStartFeeding.PT(10);

    if (lxJogPos)
    {
      liStep = 1000;
    }
    else if (lxJogNeg)
    {
      liStep = 2000;
    }
    else if (TonStartFeeding.Q())
    {
      liStep = 100;
    }
    break;

  //***************************************
  // feed filament 30mm
  //***************************************
  case 100:
    if (lxEntryAction)
    {

      extruder->moveTo(-28000 * 2);

      lxEntryAction = false;
    }

    if (true)
    {
      liStep = 110;
    }
    break;

  //***************************************
  // feed filament 30mm
  //***************************************
  case 110:
    if (lxEntryAction)
    {
      lxEntryAction = false;
    }

    if (ixEndposPos)
    {
      liStep = 0;
    }
    else if (extruder->stepsToStop() == 0)
    {
      liStep = 20;
    }
    break;

  //***************************************
  // jog positive
  //***************************************
  case 1000:
    if (lxEntryAction)
    {
      extruder->runForward();

      lxEntryAction = false;
    }

    TonStep.IN(!lxJogPos);
    TonStep.PT(50);

    if (TonStep.Q())
    {
      liStep = 0;
    }
    break;

  //***************************************
  // jog negative
  //***************************************
  case 2000:
    if (lxEntryAction)
    {
      extruder->runBackward();

      lxEntryAction = false;
    }

    TonStep.IN(!lxJogNeg);
    TonStep.PT(50);

    if (TonStep.Q())
    {
      liStep = 0;
    }
    break;
  }
}

void debugStepChange()
{
  if (liStep != liStepOld)
  {
    liStepOld = liStep;
    lxEntryAction = true;
    TonStep.IN(false);
    TonStepF.IN(false);
    debug("Aktueller Schritt: ");
    debugln(liStep);
  }
}
