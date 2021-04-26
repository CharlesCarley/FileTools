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
#include "Blender.h"
#include "ftBlend.h"
#include "ftLogger.h"
#include "ftTable.h"
#include "gtest/gtest.h"
using namespace Blender;
using namespace ftFlags;

SKhash reportFilter[]{
    ftCharHashKey("Object").hash(),
    ftCharHashKey("Mesh").hash(),
    ftCharHashKey("Camera").hash(),
    ftCharHashKey("CollectionChild").hash(),
    ftCharHashKey("CollectionObject").hash(),
    ftCharHashKey("Collection").hash(),
    0};

GTEST_TEST(BlendFile, BasicLoad)
{
    ftBlend fp;
    fp.setFileFlags(LF_DIAGNOSTICS | LF_DO_CHECKS | LF_DUMP_CAST);
    fp.setCastFilter(reportFilter, sizeof(reportFilter) / sizeof(SKhash));
    const int status = fp.load("Test.blend");
    EXPECT_EQ(FS_OK, status);
    EXPECT_EQ(0, status);
}

GTEST_TEST(BlendFile, ExtractScene)
{
    ftBlend   fp;
    const int status = fp.load("Test.blend");
    EXPECT_EQ(FS_OK, status);

    EXPECT_NE(fp.m_fg, nullptr);
    EXPECT_NE(fp.m_fg->curscene, nullptr);
    EXPECT_TRUE(strcmp(fp.m_fg->curscene->id.name, "SCScene") == 0);
}

GTEST_TEST(BlendFile, MissingDNA1)
{
    ftBlend fp;
    // This file has it's DNA1 code deliberately removed.
    const int status = fp.load("MissingDNA1.blend");
    EXPECT_EQ(FS_DNA1_READ, status);
}

GTEST_TEST(BlendFile, InvalidDNASize)
{
    ftBlend fp;
    // This file has it's DNA1 length field zeroed.
    const int status = fp.load("InvalidDNASize.blend");
    EXPECT_EQ(FS_INV_READ, status);
}

GTEST_TEST(BlendFile, DNA1WrongSize)
{
    ftBlend fp;
    // This file has it's DNA1 length field divided by 2.
    const int status = fp.load("DNA1WrongSize.blend");
    EXPECT_EQ(FS_INV_VALUE, status);
}

GTEST_TEST(BlendFile, CorruptChunks)
{
    // This file has had the OB chunks zeroed.
    ftBlend   fp;
    const int status = fp.load("CorruptChunks.blend");
    EXPECT_EQ(FS_OK, status);
}

GTEST_TEST(BlendFile, IterateObjects)
{
    ftBlend fp;
    int     status = fp.load("Test.blend");
    EXPECT_EQ(FS_OK, status);

    Scene* sc = fp.m_fg->curscene;
    EXPECT_NE(sc->master_collection, nullptr);

    ListBase         lb = sc->master_collection->children;
    CollectionChild* cc = (CollectionChild*)lb.first;
    EXPECT_NE(cc, nullptr);

    if (cc)
    {
        EXPECT_NE(cc->collection, nullptr);
        EXPECT_TRUE(strcmp(cc->collection->id.name, "GRCollection") == 0);

        ListBase collections = cc->collection->gobject;

        CollectionObject* co = (CollectionObject*)collections.first;

        int i = 0;
        while (co)
        {
            EXPECT_NE(co->ob, nullptr);
            Object* obj = co->ob;

            switch (i)
            {
            case 0:
            {
                EXPECT_TRUE(strcmp(obj->id.name, "OBCube") == 0);
                EXPECT_TRUE(obj->type == 1);
                EXPECT_TRUE(obj->data != nullptr);

                Mesh* me = (Mesh*)obj->data;
                EXPECT_TRUE(strcmp(me->id.name, "MECube") == 0);

                break;
            }
            case 1:
            {
                EXPECT_TRUE(strcmp(obj->id.name, "OBLight") == 0);
                EXPECT_TRUE(obj->data != nullptr);

                Lamp* ca = (Lamp*)obj->data;
                EXPECT_TRUE(strcmp(ca->id.name, "LALight") == 0);
                break;
            }
            case 2:
            {
                EXPECT_TRUE(strcmp(obj->id.name, "OBCamera") == 0);
                EXPECT_TRUE(obj->type == 11);
                EXPECT_TRUE(obj->data != nullptr);

                Camera* ca = (Camera*)obj->data;
                EXPECT_TRUE(strcmp(ca->id.name, "CACamera") == 0);
                break;
            }
            default:
                break;
            }

            ++i;
            co = co->next;
        }
        EXPECT_EQ(cc->next, nullptr);
    }
}

GTEST_TEST(BlendFile, AssertTypes)
{
    ftBlend   fp;
    const int status = fp.load("Test.blend");
    EXPECT_EQ(FS_OK, status);

    ftTable* table = fp.getFileTable();
    EXPECT_NE(table, nullptr);

    const ftTable::Types& types = table->getTypes();
    const SKuint32        nr    = table->getNumberOfTypes();

    for (SKuint32 i = 0; i < nr; ++i)
    {
        const ftType& type = types[i];
        if (i < table->getFirstStructType())
            EXPECT_EQ(SK_NPOS32, type.id);
        else
        {
            if (type.id != SK_NPOS32)
            {
                EXPECT_TRUE(type.id < table->getNumberOfStructs());
            }
        }
    }
}
