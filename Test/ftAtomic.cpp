/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

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
#include "gtest/gtest.h"

GTEST_TEST(ftAtomic, ftCharHashKey)
{
    ftAtomicUtils::Types;

    // test for unique hashes
    for (size_t i = 0; i < ftAtomicUtils::NumberOfTypes; ++i)
    {
        const ftAtomicType& type1 = ftAtomicUtils::Types[i];

        for (size_t j = 0; j < ftAtomicUtils::NumberOfTypes; ++j)
        {
            if (j != i)  // excluding the same idx
            {
                const ftAtomicType& type2 = ftAtomicUtils::Types[j];
                const char*         fmtString;
#if SK_ARCH == SK_ARCH64
                fmtString = "ftAtomicType (0x%016llX, 0x%016llX)\n";
#else
                fmtString = "ftAtomicType (0x%08X, 0x%08X)\n";
#endif
                printf(fmtString, type1.m_hash, type2.m_hash);
                EXPECT_NE(type1.m_hash, type2.m_hash);
            }
        }
    }
}

class TestCast
{
private:
    int    x, y, z, p;
    double d;

public:
    TestCast()
    {
        x = y = z = 0, p = 0;
        d = 0;
    }

    int getX() const
    {
        return x;
    }

    int getY() const
    {
        return y;
    }

    int getZ() const
    {
        return z;
    }

    double getD() const
    {
        return d;
    }
};

GTEST_TEST(ftAtomic, Cast)
{
    TestCast* t  = new TestCast();
    char*     cp = (char*)t;

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

GTEST_TEST(ftAtomic, Swap)
{
    const SKsize srcElmSize = 8;
    
    FBTByteInteger seed = {};
    seed.m_byte[0]      = 51;
    seed.m_byte[1]      = 26;
    seed.m_byte[2]      = 37;
    seed.m_byte[3]      = 98;
    seed.m_byte[4]      = 89;
    seed.m_byte[5]      = 73;
    seed.m_byte[6]      = 62;
    seed.m_byte[7]      = 15;

    SKuint64 i64 = seed.m_ptr;

    SKbyte  dstBuffer[ftEndianUtils::MaxSwapSpace];
    SKbyte* srcBPtr = (SKbyte*)&i64;

    memcpy(dstBuffer, srcBPtr, skMin(ftEndianUtils::MaxSwapSpace, srcElmSize));
    FBTByteInteger before, after;
    before.m_ptr = i64;

    ftEndianUtils::swap64((SKuint64*)&dstBuffer[0], 1);

    after.m_ptr = *(SKuint64*)dstBuffer;

    EXPECT_EQ(before.m_byte[0], after.m_byte[7]);
    EXPECT_EQ(before.m_byte[1], after.m_byte[6]);
    EXPECT_EQ(before.m_byte[2], after.m_byte[5]);
    EXPECT_EQ(before.m_byte[3], after.m_byte[4]);
}
