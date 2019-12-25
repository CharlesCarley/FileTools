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
#include "stdio.h"

using namespace Blender;
using namespace ftFlags;



// Unneeded structures
static FBThash skipList[] =
    {
        ftCharHashKey("FileGlobal").hash(),
        ftCharHashKey("Object").hash(),
        ftCharHashKey("Camera").hash(),
        ftCharHashKey("Lamp").hash(),
        ftCharHashKey("Scene").hash(),
        ftCharHashKey("Mesh").hash(),
        ftCharHashKey("MVert").hash(),
        ftCharHashKey("MFace").hash(),
        ftCharHashKey("MPoly").hash(),
        ftCharHashKey("MLoop").hash(),
        ftCharHashKey("Link").hash(),
        0,
};

int main(int argc, char** argv)
{
    if (argc < 2)
        return 1;

    ftBlend fp;
    fp.addFileFlag(LF_DIAGNOSTICS | LF_DO_CHECKS);
    fp.setFilterList(skipList, sizeof(skipList) / sizeof(FBThash), true);
    if (fp.load(argv[argc - 1], PM_COMPRESSED) != FS_OK)
        return 1;

    Blender::FileGlobal* fg = fp.m_fg;

    printf("Blender file version %i\n", fg->minversion);
    printf("Objects:\n");

    ftList& objects = fp.m_object;
    for (Object* ob = (Object*)objects.first; ob; ob = (Object*)ob->id.next)
    {
        printf("     Name      : %s\n", ob->id.name + 2);
        printf("     Location  : %.02f %.02f %.02f\n", ob->loc[0], ob->loc[1], ob->loc[2]);
        printf("     Rotation  : %.02f %.02f %.02f\n", ob->rot[0], ob->rot[1], ob->rot[2]);
        printf("     Scale     : %.02f %.02f %.02f\n", ob->size[0], ob->size[1], ob->size[2]);
        printf("\n");
    }

    printf("Mesh:\n");

    ftList& mesh = fp.m_mesh;
    for (Mesh* me = (Mesh*)mesh.first; me; me = (Mesh*)me->id.next)
    {
        printf("     Name                : %s\n", me->id.name + 2);
        printf("     Total Vertices      : %i\n", me->totvert);
        printf("     Total Faces         : %i\n", me->totface);
        printf("     Total Polygons      : %i\n", me->totpoly);
        if (me->mface)
        {
            for (int v = 0; v < me->totvert; ++v)
            {
                float* fp = &me->mvert[v].co[0];
                printf("         Coordinate %.02f, %.02f, %.02f \n",
                       (fp[0]),
                       (fp[1]),
                       (fp[2]));
            }
        }
        else if (me->mpoly && me->mloop)
        {
            for (int f = 0; f < me->totpoly; ++f)
            {
                printf("     Polygon %i:\n", f);
                Blender::MPoly& cp = me->mpoly[f];
                for (int i = 0; i < cp.totloop; ++i)
                {
                    float* fp = &me->mvert[me->mloop[cp.loopstart + i].v].co[0];
                    printf("         Coordinate %.02f, %.02f, %.02f \n",
                           (fp[0]),
                           (fp[1]),
                           (fp[2]));
                }
            }
        }
        printf("\n");
    }
    return 0;
}
