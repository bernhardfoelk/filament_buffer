#include <EdgePosNeg.h>

EdgePosNeg::EdgePosNeg()
    : lxIN(false), lxInOld(false), lxEdgePos(false), lxEdgeNeg(false)
{
}

void EdgePosNeg::run(bool ixIN)
{
    lxIN = ixIN;
    lxEdgePos = false;
    lxEdgeNeg = false;
    if (lxIN != lxInOld)
    {
        lxInOld = lxIN;
        if (lxIN)
        {
            lxEdgePos = true;
            lxEdgeNeg = false;
        }
        else
        {
            lxEdgePos = false;
            lxEdgeNeg = true;
        }
    }
}

bool EdgePosNeg::EdgePos() const
{
    return lxEdgePos;
}
bool EdgePosNeg::EdgeNeg() const
{
    return lxEdgeNeg;
}