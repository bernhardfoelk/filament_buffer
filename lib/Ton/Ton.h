#ifndef TON_H
#define TON_H

#include "EdgePosNeg.h"

class Ton
{
public:
    Ton(); // Standardkonstruktor
    void IN(bool ixIN);
    void PT(unsigned long ilPT); // setzt die Zeitvorgabe (ms)
    void run();                  // muss regelmäßig aufgerufen werden
    bool Q() const;              // gibt den Ausgang zurück
    unsigned long ET() const;    // gibt die aktuelle Laufzeit zurück

private:
    bool lxIN;                 // Eingang
    bool lxQ;                  // Ausgang
    unsigned long llPT;        // Vorgabezeit
    unsigned long llStartTime; // Zeitpunkt an dem Timer gestartet wurde

    EdgePosNeg Edge;
};

#endif