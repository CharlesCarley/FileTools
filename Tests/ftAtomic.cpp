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
#include "ftAtomic.h"
#include "Templates.h"
#include "catch/Macro.h"


TEST_CASE("ftAtomic ftCharHashKey")
{
    ftAtomicUtils::Types;
    size_t i, j;

    // test for unique hashes
    for (i = 0; i < ftAtomicUtils::NumberOfTypes; ++i)
    {
        const ftAtomicType& type1 = ftAtomicUtils::Types[i];

        for (j = 0; j < ftAtomicUtils::NumberOfTypes; ++j)
        {
            if (j != i)  // excluding the same idx
            {
                const ftAtomicType& type2 = ftAtomicUtils::Types[j];
                printf("ftAtomicType (0x%08X, 0x%08X)\n", type1.m_hash, type2.m_hash);
                EXPECT_NE(type1.m_hash, type2.m_hash);
            }
        }
    }
}

class Test
{
private:
    int    x, y, z, p;
    double d;

public:
    Test()
    {
        x = y = z = 0, p = 0;
        d = 0;
    }

    int getX()
    {
        return x;
    }

    int getY()
    {
        return y;
    }

    int getZ()
    {
        return z;
    }

    double getD()
    {
        return d;
    }
};



TEST_CASE("ftAtomic Cast")
{
    Test* t  = new Test();
    char* cp = (char*)t;


    int    x = 1, y = 2, z = 3;
    double d = 3.14;

    ftAtomicUtils::cast((char*)&x,
                        cp,
                        ftAtomic::FT_ATOMIC_INT,
                        ftAtomic::FT_ATOMIC_INT,
                        1);
    EXPECT_EQ(t->getX(), 1);
    EXPECT_EQ(t->getY(), 0);
    EXPECT_EQ(t->getZ(), 0);

    ftAtomicUtils::cast((char*)&y,
                        0,
                        cp,
                        4,
                        ftAtomic::FT_ATOMIC_INT,
                        ftAtomic::FT_ATOMIC_INT,
                        1);
    ftAtomicUtils::cast((char*)&z,
                        0,
                        cp,
                        8,
                        ftAtomic::FT_ATOMIC_INT,
                        ftAtomic::FT_ATOMIC_INT,
                        1);

    ftAtomicUtils::cast((char*)&d,
                        0,
                        cp,
                        16,
                        ftAtomic::FT_ATOMIC_DOUBLE,
                        ftAtomic::FT_ATOMIC_DOUBLE,
                        1);
    EXPECT_EQ(t->getX(), 1);
    EXPECT_EQ(t->getY(), 2);
    EXPECT_EQ(t->getZ(), 3);
    EXPECT_EQ(t->getD(), 3.14);
}

#include "ftEndianUtils.h"



 typedef union Split {
    FBTuint8   lohi8[8];
    FBTuint16  lohi16[4];
    FBTuint32  lohi32[2];
    FBTuint64  space;
 } Split;


 
 TEST_CASE("Swap")
 {
     FBTsize srcElmSize = 8;
     FBTsize dstElmSize = 8;


     Split seed;
     seed.lohi8[0] = 51;
     seed.lohi8[1] = 26;
     seed.lohi8[2] = 37;
     seed.lohi8[3] = 98;
     seed.lohi8[4] = 89;
     seed.lohi8[5] = 73;
     seed.lohi8[6] = 62;
     seed.lohi8[7] = 15;

     FBTuint64 i64 = seed.space;

     FBTbyte  dstBuffer[ftEndianUtils::MaxSwapSpace];
     FBTbyte* srcBPtr = (FBTbyte*)&i64;

     ::memcpy(dstBuffer, srcBPtr, skMin(ftEndianUtils::MaxSwapSpace, srcElmSize));
     Split before, after;
     before.space = i64;

     ftEndianUtils::swap64((FBTuint64*)dstBuffer, 1);

     after.space = *((FBTuint64*)dstBuffer);

     EXPECT_EQ(before.lohi8[0], after.lohi8[7]);
     EXPECT_EQ(before.lohi8[1], after.lohi8[6]);
     EXPECT_EQ(before.lohi8[2], after.lohi8[5]);
     EXPECT_EQ(before.lohi8[3], after.lohi8[4]);
 }