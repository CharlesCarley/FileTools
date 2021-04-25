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
#ifndef _ftLogger_h_
#define _ftLogger_h_

#include "Utils/skDebugger.h"
#include "ftFile.h"
#include "ftTable.h"

namespace ftLogger
{
    /// <summary>
    /// Logs the enumerated name a predetermined status return code.
    /// </summary>
    /// <param name="status">code defined in <see cref="ftFlags::FileStatus">ftFlags::FileStatus</see></param>
    extern void log(int status);

    extern void log(int status, const char *msg, ...);


    extern void logF(const char *msg, ...);

    extern void separator();
    extern void divider();
    extern void newline(int nr = 1);
    extern void color(skConsoleColorSpace cs);

    extern void log(const ftChunk &chunk);
    extern void log(const void *ptr, const SKsize& len);
    extern void log(ftStruct *strc);
    extern void log(ftMember *strc);
    extern void log(ftStruct *fstrc, ftStruct *mstrc);
    extern void log(const ftName &name);
    extern void log(const ftType &type);
    extern void log(const ftType &type, FTtype size);


    extern void logDiagnosticsCastHeader(const ftChunk &chunk,
                                         ftStruct *     fstrc,
                                         ftStruct *     mstrc);
    extern void logDiagnosticsCastMemberHeader(ftMember *fstrc,
                                               ftMember *mstrc);

    extern void logReadChunk(const ftChunk &chunk,
                             const void *   block,
                             const SKsize &len);

    extern void logSkipChunk(const ftChunk &chunk,
                             ftStruct *     fstrc,
                             const void *   block,
                             const SKsize &len);


    extern void logUnresolvedStructure(ftMemoryChunk *bin, ftStruct *fstrc, ftStruct *mstrc);

    extern void logInvalidInsert(ftMemoryChunk *bin);

}  // namespace ftLogger


#endif  //_ftLogger_h_