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
#define CATCH_CONFIG_MAIN
#include "Blender.h"
#include "catch/catch.hpp"
#include "catch/defines.h"
#include "ftBlend.h"
#include "stdio.h"
using namespace Blender;
using namespace ftFlags;


FBThash castRept[]
{
    ftCharHashKey("Object").hash(),
    ftCharHashKey("Mesh").hash(),
    ftCharHashKey("Camera").hash(),
    ftCharHashKey("CollectionChild").hash(),
    ftCharHashKey("CollectionObject").hash(),
    ftCharHashKey("Collection").hash(),
    0
};

TEST_CASE("Basic_load")
{
    ftBlend fp;
    fp.setFileFlags(ftFlags::LF_DIAGNOSTICS|ftFlags::LF_DO_CHECKS|ftFlags::LF_DUMP_CAST);
    fp.setCastFilter(castRept, sizeof(castRept)/sizeof(FBThash));
    int     status = fp.load("Test.blend");
    EXPECT_EQ(FS_OK, status);
}


TEST_CASE("SceneGrab")
{
    ftBlend fp;
    int     status = fp.load("Test.blend");
    EXPECT_EQ(FS_OK, status);

    EXPECT_NE(fp.m_fg, nullptr);
    EXPECT_NE(fp.m_fg->curscene, nullptr);
    EXPECT_TRUE(strcmp(fp.m_fg->curscene->id.name, "SCScene") == 0);
}



TEST_CASE("SceneIterate")
{
    ftBlend fp;
    int     status = fp.load("Test.blend");
    EXPECT_EQ(FS_OK, status);

    Blender::Scene *sc = fp.m_fg->curscene;
    EXPECT_NE(sc->master_collection, nullptr);

    ListBase         lb = sc->master_collection->children;
    CollectionChild *cc = (CollectionChild *)lb.first;
    EXPECT_NE(cc, nullptr);

    if (cc)
    {
        EXPECT_NE(cc->collection, nullptr);
        EXPECT_TRUE(strcmp(cc->collection->id.name, "GRCollection") == 0);

        ListBase          colb = cc->collection->gobject;
        CollectionObject *co   = (CollectionObject *)colb.first;

        int i = 0;
        while (co)
        {
            EXPECT_NE(co->ob, nullptr);
            Blender::Object *obj = co->ob;

            switch (i)
            {
            case 0:
            {
                EXPECT_TRUE(strcmp(obj->id.name, "OBCube") == 0);
                EXPECT_TRUE(obj->type == 1);
                EXPECT_TRUE(obj->data != nullptr);

                Blender::Mesh *me = (Blender::Mesh *)obj->data;
                EXPECT_TRUE(strcmp(me->id.name, "MECube") == 0);

                break;
            }
            case 1:
            {
                EXPECT_TRUE(strcmp(obj->id.name, "OBLight") == 0);
                EXPECT_TRUE(obj->data != nullptr);

                Blender::Lamp *ca = (Blender::Lamp *)obj->data;
                EXPECT_TRUE(strcmp(ca->id.name, "LALight") == 0);
                break;
            }
            case 2:
            {
                EXPECT_TRUE(strcmp(obj->id.name, "OBCamera") == 0);
                EXPECT_TRUE(obj->type == 11);
                EXPECT_TRUE(obj->data != nullptr);

                Blender::Camera *ca = (Blender::Camera *)obj->data;
                EXPECT_TRUE(strcmp(ca->id.name, "CACamera") == 0);
                break;
            }
            }

            ++i;
            co = co->next;
        }
        EXPECT_EQ(cc->next, nullptr);
    }
}
