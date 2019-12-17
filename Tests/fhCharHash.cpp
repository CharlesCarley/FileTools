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
#include <string>
#include <vector>
#include "Templates/LargeStringArray.h"
#include "Utils/skMap.h"
#include "catch/Macro.h"
#include "ftAtomic.h"
#include "ftHashTypes.h"

typedef skHashTable<ftCharHashKey, SKuint32> TypeFinder;
typedef skHashTable<SKuintPtr, SKuintPtr>    PointerFinder;


TEST_CASE("ftCharHash")
{
    TypeFinder find_type;
    size_t     i = 0;
    for (i = 0; i < random_names_len; ++i)
    {
        ftCharHashKey chk(random_names[i]);

        SKsize lookup = find_type.find(chk);
        EXPECT_EQ(SK_NPOS, lookup);

        bool result = find_type.insert(chk, (SKuint32)i + 1);
        EXPECT_EQ(true, result);
    }

    EXPECT_EQ(random_names_len, find_type.size());
    i = 0;
    
    TypeFinder::Iterator it = find_type.iterator();
    while (it.hasMoreElements() && i < random_names_len)
    {
        ftCharHashKey chk = it.getNext().first;

        size_t cmp = strcmp(chk.key(), random_names[i]);
        EXPECT_EQ(0, cmp);
        ++i;
    }
}



TEST_CASE("PointerHash")
{
    int *ptrArray = new int[101];

    size_t i = 0;
    for (i = 0; i < 100; ++i)
        ptrArray[i] = (int)i + 1;


    PointerFinder find_ptr;

    int *it = ptrArray;

    for (i = 0; i < 100; ++i)
    {
        SKuintPtr ptr = (SKuintPtr)(it + i);

        SKsize lookup = find_ptr.find(ptr);
        EXPECT_EQ(SK_NPOS, lookup);


        bool result = find_ptr.insert(ptr, it[i]);
        EXPECT_EQ(true, result);
    }

    EXPECT_EQ(100, find_ptr.size());

    i = 0;

    PointerFinder::Iterator pit = find_ptr.iterator();
    while (pit.hasMoreElements() && i < 100)
    {
        SKuintPtr chk = pit.getNext().first;
        SKuintPtr ptr = (SKuintPtr)(it + i);
        EXPECT_EQ(ptr, chk);

        ++i;
    }

    delete[] ptrArray;
}
