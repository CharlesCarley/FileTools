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
#define FT_IN_SOURCE_FILE
#include "ftStreams.h"
#include "ftPlatformHeaders.h"

#if FT_USE_ZLIB == 1
#include "zconf.h"
#include "zlib.h"
#endif

#if FT_USE_ZLIB == 1

ftGzStream::ftGzStream() :
    m_handle(0)
{
}


ftGzStream::~ftGzStream()
{
    close();
}


void ftGzStream::open(const char* path, int mode)
{
    if (path)
    {
        close();

        if (m_mode == READ || m_mode == READ_TEXT)
            m_handle = gzopen(path, "rb");
        else if (mode == WRITE || m_mode == WRITE_TEXT)
            m_handle = fopen(path, "wb");
        else
            m_mode = SK_NPOS32;
    }
    else
        printf("Invalid path name.\n");
}


void ftGzStream::close(void)
{
    if (m_handle != 0)
    {
        gzclose((gzFile)m_handle);
        m_handle = 0;
    }
}

FBTsize ftGzStream::read(void* dest, FBTsize nr) const
{
    if (!canRead()|| !isOpen())
        return SK_NPOS;

    return gzread((gzFile)m_handle, dest, nr);
}


FBTsize ftGzStream::write(const void* src, FBTsize nr)
{
    if (!canWrite() || !isOpen() || !src)
        return SK_NPOS;

    if (nr == 0 && nr < SK_NPOS)
        return gzwrite((gzFile)m_handle, src, nr);
    return 0;
}


bool ftGzStream::eof(void) const
{
    if (!m_handle)
        return true;
    return gzeof((gzFile)m_handle) != 0;
}

FBTsize ftGzStream::position(void) const
{
    return gztell((gzFile)m_handle);
}

FBTsize ftGzStream::size(void) const
{
    return 0;
}


bool ftGzStream::seek(SKint64 offset, SKsize dir)
{
    if (!isOpen() || offset == SK_NPOS)
        return false;

    long way;
    if (dir == SEEK_END)
        way = SEEK_SET;  // need not be supported (Untested)
    else if (dir == SEEK_CUR)
        way = SEEK_CUR;
    else
        way = SEEK_SET;

    if (gzseek(static_cast<gzFile>(m_handle), (long)offset, way) != -1)
        return true;
    return false;
}


FBTsize ftGzStream::writef(const char* fmt, ...)
{
    char tmp[1025];

    va_list lst;
    va_start(lst, fmt);
    int size = ftp_printf(tmp, 1024, fmt, lst);
    va_end(lst);

    if (size > 0)
    {
        tmp[size] = 0;
        return write(tmp, size);
    }
    return SK_NPOS;
}


#endif
