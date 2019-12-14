#include "ftTypes.h"
#include "ftHashTypes.h"



FBTuint32 skHash(const ftCharHashKey& hk)
{
    return hk.hash();
}
