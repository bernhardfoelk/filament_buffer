//
// Nicht blockierender, modulo-zyklischer Schrittmotor-Treiber auf Basis von
// AccelStepper. Die Sollposition wird in Grad (0..360) angegeben; die Achse
// bewegt sich IMMER nur vorwärts und läuft modulo einer vollen Umdrehung
// (z.B. für Ziffern-/Skalenscheiben, drehbare Anzeigen).
//

#ifndef STEPPERDRIVERABSOLUT_H
#define STEPPERDRIVERABSOLUT_H

#include "EdgePosNeg.h"
#include <AccelStepper.h>

class StepperDriverAbsolut {
public:
    // byStepPin/byDirPin: Ansteuerung des Treibers
    // byRefPin:            Referenzschalter dieser Achse (Position 0°)
    // diSchritteProUmdrehung: volle Schritte für eine Umdrehung
    //                         (Motor-Vollschritte * Mikroschritte * Getriebeuntersetzung)
    StepperDriverAbsolut(uint8_t iiStepPin, uint8_t iiDirPin,
                         long ilStepsPerRotation);

    // ---------------- Eingänge ----------------
    void setTargetPosition(float irValue); // Zielposition in Grad (0..360, wird moduliert)
    void setStart(); // steigende Flanke startet Drehung zur Sollposition
    void setStartReference(); // steigende Flanke startet Referenzfahrt
    void setReference(long ilValue); // setzt Ist-Position auf eingegebenen Wert

    // ---------------- Parametrierung (optional) ----------------
    void setSpeed(float irValue); // [Schritte/s]
    void setAcceleration(float irValue); // [Schritte/s^2]
    void setSpeedReference(float irValue); // Geschwindigkeit bei der Referenzfahrt
    void setOffsetToReference(long ilValue); // Korrektur [Schritte] zw. Schalterposition und 0°

    // Muss zyklisch aufgerufen werden (z.B. in jedem loop()-Durchlauf). Blockiert nie.
    void run();

    void reset();

    // ---------------- Ausgänge ----------------
    bool isFinished() const; // Fertig-Bit: aktuell keine Fahrt/Referenzierung aktiv
    bool isReady() const; // Bereit-Bit: Baustein ist frei, kann neuen Auftrag annehmen
    bool isReferenced() const; // Referenziert-Bit: Nullposition wurde erfolgreich gefunden

private:
    AccelStepper fbStepper;

    EdgePosNeg EdgeStart;
    EdgePosNeg EdgeStartReference;
    EdgePosNeg SetReference;

    long llOffsetToReference;

    float lfStepsPerRotation;
    float lfStepsPerMm;

    float lrTargetPosition;
    float lrSpeed;
    float lrReferenceSpeed;

    bool lxStart;
    bool lxStartReference;
    bool lxSetReference;

    bool lxReady;
    bool lxFinished;
    bool lxReferenced;
    int liStep;

    long llTargetStep;
    long llReferencePosition;

    long MmToSteps(float irMm); // interne Hilfsfunktion
};

#endif
