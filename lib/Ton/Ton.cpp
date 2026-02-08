#include <Ton.h>
#include <Arduino.h>

Ton::Ton()
    : lxIN(false), 
    lxQ(false), 
    llPT(1000), 
    llStartTime(0), 
    Edge()
{
}

void Ton::PT(unsigned long ilPT)
{
    llPT = ilPT;
}

void Ton::IN(bool ixIN)
{
    lxIN = ixIN;
}

void Ton::run()
{

    Edge.run(lxIN);

    if (Edge.EdgePos())
    {
        // steigende Flanke -> Startzeit merken
        llStartTime = millis();
    }

    if (lxIN)
    {
        if ((millis() - llStartTime) >= llPT)
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
        llStartTime = millis();
    }
}

bool Ton::Q() const
{
    return lxQ;
}

unsigned long Ton::ET() const
{
    unsigned long tlElapsed = 0;
    if (lxIN)
    {
        tlElapsed = millis() - llStartTime;
    }
    return (tlElapsed > llPT) ? llPT : tlElapsed; // begrenzen
}