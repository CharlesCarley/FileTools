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
#include "Templates.h"
#include "Utils/skHexPrint.h"
#include "Utils/skMemoryStream.h"
#include "catch/Macro.h"
#include "ftAtomic.h"
#include "ftCompiler.h"
#include "ftLogger.h"
#include "ftScanner.h"



TEST_CASE("CompilerTest")
{
    int        status;
    ftCompiler compiler;
    status = compiler.parseBuffer("TestGen", (const char*)TETSTAPI, TETSTAPI_SIZE);
    EXPECT_GE(status, 0);

    status = compiler.buildTypes();

    EXPECT_EQ(LNK_OK, status);
    EXPECT_EQ(compiler.getNumberOfBuiltinTypes(), ftAtomicUtils::NumberOfTypes);



    compiler.setWriteMode(ftCompiler::WRITE_STREAM);
    skMemoryStream stream;
    compiler.writeStream(&stream);

    ftLogger::log(stream.ptr(), stream.size());


    ftBinTables tbl;
    tbl.read(stream.ptr(), stream.size(), false);

    FBTuint32 nr, i, j, mnr;

    nr = tbl.getNumberOfTypes();
    i  = 0;


    while (i < nr)
    {
        const ftType& type = tbl.getTypeAt(i);

        EXPECT_NE(nullptr, type.m_name);
        EXPECT_NE(SK_NPOS, type.m_typeId);

        // builtin types do not point to a structure
        if (i < compiler.getNumberOfBuiltinTypes())
            EXPECT_EQ(type.m_strcId, SK_NPOS32);
        else
            EXPECT_LT(type.m_strcId, tbl.getNumberOfStructs());

        ++i;
    }


    nr = tbl.getNumberOfNames();
    i  = 0;
    while (i < nr)
    {
        const ftName& name = tbl.getNameAt(i);

        EXPECT_NE(nullptr, name.m_name);

        ftCharHashKey hk(name.m_name);
        EXPECT_EQ(name.m_hashedName, hk.hash());
        ++i;
    }


    nr = tbl.getOffsetCount();
    i  = 0;
    while (i < nr)
    {
        ftStruct* src = tbl.getOffsetPtr()[i];
        mnr           = src->getMemberCount();

        ftLogger::log(src);

        j = 0;
        while (j < mnr)
        {


            //ftLogger::log(&tbl, mbr);

            ++j;
        }

        ++i;
    }
}
