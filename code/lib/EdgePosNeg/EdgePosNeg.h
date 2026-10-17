#ifndef EdgePosNeg_H
#define EdgePosNeg_H

class EdgePosNeg
{
public:
    EdgePosNeg();
    void run(bool ixIN);
    bool EdgePos() const;
    bool EdgeNeg() const;

private:
    bool lxIN;    // Eingang
    bool lxInOld; // Merker Eingang

    bool lxEdgePos; // Ausgang positive Flanke
    bool lxEdgeNeg; // Ausgang negative Flanke
};

#endif