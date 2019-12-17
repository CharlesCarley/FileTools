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
#ifndef _ftBlend_h_
#define _ftBlend_h_

#include "ftFile.h"
#include "Blender.h"

class ftBlend : public ftFile
{
public:
    ftBlend();
    virtual ~ftBlend();

    ftList m_scene;
    ftList m_library;
    ftList m_object;
    ftList m_mesh;
    ftList m_curve;
    ftList m_mball;
    ftList m_mat;
    ftList m_tex;
    ftList m_image;
    ftList m_latt;
    ftList m_lamp;
    ftList m_camera;
    ftList m_ipo;
    ftList m_key;
    ftList m_world;
    ftList m_screen;
    ftList m_script;
    ftList m_vfont;
    ftList m_text;
    ftList m_sound;
    ftList m_group;
    ftList m_armature;
    ftList m_action;
    ftList m_nodetree;
    ftList m_brush;
    ftList m_particle;
    ftList m_wm;
    ftList m_gpencil;

    Blender::FileGlobal* m_fg;

    int save(const char* path, const int mode = PM_UNCOMPRESSED);

    void setFilterList(FBTuint32* filter, bool inclusive = false);

protected:
    virtual bool skip(const FBTuint32& id);

    virtual int notifyDataRead(void* p, const Chunk& id);
    virtual int serializeData(skStream* stream);

    FBTuint32*      m_filterList;
    FBTint32        m_filterListLen;
    bool            m_inclusive;

    virtual void*   getTables(void);
    virtual FBTsize getTableSize(void);
};


#endif//_ftBlend_h_
