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
bool lxFilamentPresent = false;
bool lxEndposPos = false;
bool lxEndposNeg = false;
bool lxFeedForw = false;
bool lxFeedBackw = false;

// Stepper motor
Stepper Extruder;

// Timer
Ton TonStep;
Ton TonStepF;
Ton TonFilamentSensor;
Ton TonStartFeeding;
Ton TonFeedForw;
Ton TonFeedBackw;

// Edge detection for buttons
EdgePosNeg Edge_FilamentPresent;
EdgePosNeg Edge_EndstopPos;
EdgePosNeg Edge_EndstopNeg;
EdgePosNeg Edge_FeedForw;
EdgePosNeg Edge_FeedBackw;

Ton TonDebounceFilamentSensor;
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
  pinMode(PIN_FILAMENT_PRESENT, INPUT);

  // Define Stepper
  Extruder.setup(PIN_STEPPER_STEP, PIN_STEPPER_DIR, 1, 5);

  // Start serial interface
  Serial.begin(9600);
  delay(500);
  debugln("Debugging started..");
}

void loop()
{
  readInputs();
  updateEdges();
  runStateMachine();
  updateTimersAndJog();
  Extruder.run();
  debugStepChange();
}

// -------------------------------------------------------------
// Helper functions
// -------------------------------------------------------------

void readInputs()
{
  TonDebounceFilamentSensor.IN(!(digitalRead(PIN_FILAMENT_PRESENT)));
  TonDebounceFilamentSensor.PT(10);
  TonDebounceFilamentSensor.run();
  lxFilamentPresent = TonDebounceFilamentSensor.Q();

  TonDebounceEndstopPos.IN(digitalRead(PIN_ENDSTOP_POSITIVE));
  TonDebounceEndstopPos.PT(10);
  TonDebounceEndstopPos.run();
  lxEndposPos = TonDebounceEndstopPos.Q();

  TonDebounceEndstopNeg.IN(digitalRead(PIN_ENDSTOP_NEGATIVE));
  TonDebounceEndstopNeg.PT(10);
  TonDebounceEndstopNeg.run();
  lxEndposNeg = TonDebounceEndstopNeg.Q();
  
  TonDebounceFeedForw.IN(!(digitalRead(PIN_FEED_FORW_BUTTON)));
  TonDebounceFeedForw.PT(10);
  TonDebounceFeedForw.run();
  lxFeedForw = TonDebounceFeedForw.Q();
  
  TonDebounceFeedBackw.IN(!(digitalRead(PIN_FEED_BACKW_BUTTON)));
  TonDebounceFeedBackw.PT(10);
  TonDebounceFeedBackw.run();
  lxFeedBackw = TonDebounceFeedBackw.Q();
}

void updateEdges()
{
  Edge_FilamentPresent.run(lxFilamentPresent);
  Edge_EndstopPos.run(lxEndposPos);
  Edge_EndstopNeg.run(lxEndposNeg);
  Edge_FeedForw.run(lxFeedForw);
  Edge_FeedBackw.run(lxFeedBackw);

  if (Edge_FilamentPresent.EdgePos()){
    debugln("Filamentsensor: Positive Flanke!");
  }
  if (Edge_EndstopPos.EdgePos()){
    debugln("Endstop positiv: Positive Flanke!");
  }
  if (Edge_EndstopNeg.EdgePos()){
    debugln("Endstop negativ: Positive Flanke!");
  }
  if (Edge_FeedForw.EdgePos()){
    debugln("Sensor feed forward: Positive Flanke!");
  }
  if (Edge_FeedBackw.EdgePos()){
    debugln("Sensor feed backward: Positive Flanke!");
  }
}

void updateTimersAndJog()
{
  // Timer call
  TonStep.run();
  TonStep.IN(false);

  TonStepF.run();
  TonStepF.IN(false);

  TonFilamentSensor.run();
  TonFilamentSensor.IN(false);

  TonStartFeeding.run();
  TonStartFeeding.IN(false);

  // Jog buttons with delay
  TonFeedForw.IN(lxFeedForw);
  TonFeedForw.PT(500);
  TonFeedForw.run();
  lxJogPos = TonFeedForw.Q();

  TonFeedBackw.IN(lxFeedBackw);
  TonFeedBackw.PT(500);
  TonFeedBackw.run();
  lxJogNeg = TonFeedBackw.Q();
}

void runStateMachine()
{
  switch (liStep)
  {
  //***************************************
  // Reset step
  //***************************************
  case 0:
    if (lxEntryAction)
    {
      lxEntryAction = false;
    }

    Extruder.reset();
    liStep = 10;
    break;

  //***************************************
  // Wait for power on
  //***************************************
  case 10:
    if (lxEntryAction)
    {
      lxEntryAction = false;
    }

    if (Extruder.ready())
    {
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

    TonFilamentSensor.IN(lxFilamentPresent);
    TonFilamentSensor.PT(500);
    TonStartFeeding.IN(lxEndposNeg);
    TonStartFeeding.PT(10);

    if (lxJogPos)
    {
      Extruder.jogPos();
    }
    else if (lxJogNeg)
    {
      Extruder.jogNeg();
    }
    else if (TonFilamentSensor.Q() && TonStartFeeding.Q())
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
      lxEntryAction = false;
    }

    if (Extruder.ready())
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
      Extruder.setPosWay(30);    // 30mm
      Extruder.setVelocity(120); // 120mm/s
      Extruder.start(true);

      lxEntryAction = false;
    }

    if (lxEndposPos)
    {
      liStep = 0;
    }
    else if (Extruder.finished() && !Extruder.error())
    {
      Extruder.start(false);
      liStep = 20;
    }
    break;

  default:
    // Fallback, if somethnig goes wrong
    liStep = 0;
    lxEntryAction = true;
    break;
  }
}

void debugStepChange()
{
  if (liStep != liStepOld)
  {
    liStepOld = liStep;
    lxEntryAction = true;
    debug("Aktueller Schritt: ");
    debugln(liStep);
  }
}

/*

void setup()
{
  // Define button pins as input
  pinMode(PIN_FEED_FORW_BUTTON, INPUT);
  pinMode(PIN_FEED_BACKW_BUTTON, INPUT);
  pinMode(PIN_ENDSTOP_NEGATIVE, INPUT);
  pinMode(PIN_ENDSTOP_POSITIVE, INPUT);
  pinMode(PIN_FILAMENT_PRESENT, INPUT);

  // Define Stepper
  Extruder.setup(6, 7, 1, 5);

  // Start serial interface
  Serial.begin(9600);
  delay(500);
  debugln("Debugging started..");
}

void loop()
{

  // check inputs
  lxFilamentPresent = (digitalRead(PIN_FILAMENT_PRESENT));
  lxEndposPos = (digitalRead(PIN_ENDSTOP_POSITIVE));
  lxEndposNeg = (digitalRead(PIN_ENDSTOP_NEGATIVE));
  lxFeedForw = (digitalRead(PIN_FEED_FORW_BUTTON));
  lxFeedBackw = (digitalRead(PIN_FEED_BACKW_BUTTON));

  // Edge detection for buttons
  Edge_FilamentPresent.run(lxFilamentPresent);
  Edge_EndstopPos.run(lxEndposPos);
  Edge_EndstopNeg.run(lxEndposNeg);
  Edge_FeedForw.run(lxFeedForw);
  Edge_FeedBackw.run(lxFeedBackw);

  // State machine
  switch (liStep)
  {

  //***************************************
  // Reset step
  //***************************************
  case 0:
    if (lxEntryAction)
    {
      lxEntryAction = false;
    }

    Extruder.reset();

    liStep = 20;
    break;

  //***************************************
  // Wait for power on
  case 10:
    if (lxEntryAction)
    {
      lxEntryAction = false;
    }

    if (Extruder.ready())
    {

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

    TonFilamentSensor.IN(lxFilamentPresent);
    TonFilamentSensor.PT(500);
    TonStartFeeding.IN(lxEndposNeg);
    TonStartFeeding.PT(10);

    if (lxJogPos)
    {
      Extruder.jogPos();
    }
    else if (lxJogNeg)
    {
      Extruder.jogNeg();
    }
    else if (TonStep.Q() && TonStartFeeding.Q())
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
      lxEntryAction = false;
    }

    if (Extruder.ready())
    {
      liStep = 110;
    }
    break;

  //***************************************
  // feed filament 30mm
  case 110:
    if (lxEntryAction)
    {
      Extruder.setPosWay(30);    // 30mm
      Extruder.setVelocity(120); // 120mm/s

      Extruder.start(true);

      lxEntryAction = false;
    }

    if (lxEndposPos)
    {

      liStep = 0;
    }
    else if (Extruder.finished() && !Extruder.error())
    {

      Extruder.start(false);

      liStep = 20;
    }
    break;
  }

  // Timer call
  TonStep.run();
  TonStep.IN(false);

  TonStepF.run();
  TonStepF.IN(false);

  TonFilamentSensor.run();
  TonFilamentSensor.IN(false);

  TonStartFeeding.run();
  TonStartFeeding.IN(false);

  TonFeedForw.IN(lxFeedForw);
  TonFeedForw.PT(500);
  TonFeedForw.run();
  lxJogPos = TonFeedForw.Q();

  TonFeedBackw.IN(lxFeedBackw);
  TonFeedBackw.PT(500);
  TonFeedBackw.run();
  lxJogNeg = TonFeedBackw.Q();

  // Stepper call
  Extruder.run();

  // Debug: log change of step variable
  if (liStep != liStepOld)
  {
    liStepOld = liStep;
    lxEntryAction = true;
    debug("Aktueller Schritt: ");
    debugln(liStep);
  }
}
*/