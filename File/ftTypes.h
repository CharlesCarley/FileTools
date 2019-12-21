/*
-------------------------------------------------------------------------------

    Copyright (c) Charles Carley.

    Contributor(s): none yet.

-------------------------------------------------------------------------------
  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#ifndef _ftTypes_h_
#define _ftTypes_h_

#ifndef ftHAVE_CONFIG
#include "ftConfig.h"
#endif

#include "Utils/skArray.h"
#include "Utils/skFixedString.h"
#include "memory.h"
#include "string.h"

#define ftID(a, b, c, d) SK_ID(a, b, c, d)
#define ftID2(a, b) SK_ID2(a, b)
#define FT_INLINE SK_INLINE

typedef SKlong    FBTlong;
typedef SKulong   FBTulong;
typedef SKint32   FBTint32;
typedef SKuint32  FBTuint32;
typedef SKint16   FBTint16;
typedef SKuint16  FBTuint16;
typedef SKint8    FBTint8;
typedef SKuint8   FBTuint8;
typedef SKuint8   FBTubyte;
typedef SKbyte    FBTbyte;
typedef SKint64   FBTint64;
typedef SKuint64  FBTuint64;
typedef SKuintPtr FBTuintPtr;
typedef SKintPtr  FBTintPtr;
typedef SKuintPtr FBTsize;
typedef FBTsize   FBThash;
typedef FBTuint16 FBTtype;


#ifdef ftSCALAR_DOUBLE
#define scalar_t double
#else
#define scalar_t float
#endif

enum ftPointerLen
{
    FT_VOIDP = sizeof(void*),
    FT_VOID4 = FT_VOIDP == 4,
    FT_VOID8 = FT_VOIDP == 8,
};

class ftList
{
public:
    struct Link
    {
        Link* next;
        Link* prev;
    };

    Link* first;
    Link* last;

public:
    ftList() :
        first(0),
        last(0)
    {
    }
    ~ftList()
    {
        clear();
    }

    void clear(void)
    {
        first = last = 0;
    }

    void push_back(void* v)
    {
        Link* link = ((Link*)v);
        if (!link)
            return;
        link->prev = last;
        if (last)
            last->next = link;
        if (!first)
            first = link;
        last = link;
    }

    bool erase(void *vp)
    {
        Link* link = ((Link*)vp);
        if (!link)
            return false;

        if (link->next)
            link->next->prev = link->prev;
        if (link->prev)
            link->prev->next = link->next;
        if (last == link)
            last = link->prev;
        if (first == link)
            first = link->next;
        return true;
    }

    bool remove(void* vp)
    {
        return erase(vp);
    }
};


#define ftCharNEq(a, b, n) ((a && b) && !strncmp(a, b, n))
#define ftCharEq(a, b) ((a && b) && (*a == *b) && !strcmp(a, b))
#define ftCharEqL(a, b, l) ((a && b) && (*a == *b) && !memcmp(a, b, l))
#define ftStrLen(a) ::strlen(a)


#define ftFixedString skFixedString

class skStream;
class ftMemoryStream;
class ftTables;
class ftStruct;
struct ftName;
class ftMember;
class ftTables;
class ftStruct;


#endif  //_ftTypes_h_
