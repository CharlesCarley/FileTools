/*
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
#include "ftBlend.h"
#include "Blender.h"
using namespace Blender;


// Unneeded structures 
static FBTuint32 skipList[] =
{
    ftCharHashKey("Panel").hash(),
    ftCharHashKey("ARegion").hash(),
    ftCharHashKey("ScrArea").hash(),
    ftCharHashKey("ScrVert").hash(),
    ftCharHashKey("ScrEdge").hash(),
    ftCharHashKey("bScreen").hash(),
    ftCharHashKey("View3D").hash(),
    ftCharHashKey("SpaceButs").hash(),
    ftCharHashKey("SpaceOops").hash(),
    ftCharHashKey("SpaceImage").hash(),
    ftCharHashKey("SpaceIpo").hash(),
    ftCharHashKey("SpaceAction").hash(),
    ftCharHashKey("SpaceFile").hash(),
    ftCharHashKey("SpaceSound").hash(),
    ftCharHashKey("SpaceNla").hash(),
    ftCharHashKey("SpaceTime").hash(),
    ftCharHashKey("wmWindowManager").hash(),
    ftCharHashKey("wmWindow").hash(),
    ftCharHashKey("wmKeymap").hash(),
    ftCharHashKey("wmKeyConfig").hash(),
    ftCharHashKey("wmOperator").hash(),
    ftCharHashKey("ThemeUI").hash(),
    ftCharHashKey("bTheme").hash(),
    ftCharHashKey("uiStyle").hash(),
    ftCharHashKey("uiFont").hash(),
    ftCharHashKey("RenderData").hash(),
    0,
};

int main(int argc, char** argv)
{
    if (argc < 2)
        return 1;

    ftBlend fp;

    fp.setIgnoreList(skipList);
    if (fp.parse(argv[argc - 1], ftFile::PM_COMPRESSED) != ftFile::FS_OK)
        return 1;

    fp.generateTypeCastLog("log.html");

    Blender::FileGlobal* fg = fp.m_fg;
    ftPrintf("Blender file version %i\n", fg->minversion);

    ftPrintf("Objects:\n");
    ftList& objects = fp.m_object;
    for (Object* ob = (Object*)objects.first; ob; ob = (Object*)ob->id.next)
    {
        ftPrintf("     Name      : %s\n", ob->id.name + 2);
        ftPrintf("     Location  : %.02f %.02f %.02f\n", ob->loc[0],  ob->loc[1],  ob->loc[2]);
        ftPrintf("     Rotation  : %.02f %.02f %.02f\n", ob->rot[0],  ob->rot[1],  ob->rot[2]);
        ftPrintf("     Scale     : %.02f %.02f %.02f\n", ob->size[0], ob->size[1], ob->size[2]);
        ftPrintf("\n");
    }

    ftPrintf("Mesh:\n");
    ftList& mesh = fp.m_mesh;
    for (Mesh* me = (Mesh*)mesh.first; me; me = (Mesh*)me->id.next)
    {
        ftPrintf("     Name                : %s\n", me->id.name + 2);
        ftPrintf("     Total Vertices      : %i\n", me->totvert);
        ftPrintf("     Total Faces         : %i\n", me->totface);
        ftPrintf("     Total Polygons      : %i\n", me->totpoly);
        if (me->mface)
        {
            for (int f = 0; f < me->totface; ++f)
            {
            }
        }
        else if (me->mpoly)
        {
            for (int f = 0; f < me->totpoly; ++f)
            {
                ftPrintf("     Polygon %i:\n", f);
                Blender::MPoly& cp = me->mpoly[f];
                for (int i = 0; i < cp.totloop; ++i)
                {
                    float* fp = &me->mvert[me->mloop[cp.loopstart + i].v].co[0];
                    ftPrintf("         Coordinate %.02f, %.02f, %.02f \n", (*fp++), (*fp++), (*fp++));
                }
            }
        }
        ftPrintf("\n");
    }

    return 0;
}
