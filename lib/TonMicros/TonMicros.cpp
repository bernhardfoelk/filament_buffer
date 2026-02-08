#include <TonMicros.h>
#include <Arduino.h>

TonMicros::TonMicros()
    : lxIN(false), 
    lxQ(false), 
    llPT(1000), 
    llStartTime(0), 
    Edge()
{
}

void TonMicros::PT(unsigned long ilPT)
{
    llPT = ilPT;
}

void TonMicros::IN(bool ixIN)
{
    lxIN = ixIN;
}

void TonMicros::run()
{

    Edge.run(lxIN);

    if (Edge.EdgePos())
    {
        // steigende Flanke -> Startzeit merken
        llStartTime = micros();
    }

    if (lxIN)
    {
        if ((micros() - llStartTime) >= llPT)
        {
            lxQ = true;
        }
        else
        {
            lxQ = false;
        }
    }
    else
    {
        lxQ = false;
        llStartTime = micros();
    }
}

bool TonMicros::Q() const
{
    return lxQ;
}

unsigned long TonMicros::ET() const
{
    unsigned long tlElapsed;
    if (lxIN)
    {
        tlElapsed = millis() - llStartTime;
    }
    return (tlElapsed > llPT) ? llPT : tlElapsed; // begrenzen
}