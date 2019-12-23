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
#include "zlib.h"
#include "zconf.h"
#endif

#if FT_USE_ZLIB == 1

ftGzStream::ftGzStream() :
    m_handle(0),
    m_mode(0)
{
}

ftGzStream::~ftGzStream()
{
    close();
}

void ftGzStream::open(const char* p, skStream::Mode mode)
{
    if (m_handle != 0 && p != 0)
        gzclose((gzFile)m_handle);

    char fm[3] = {};
    char* mp = &fm[0];
    if (mode & READ)
        *mp++ = 'r';
    else if (mode & WRITE)
        *mp++ = 'w';
    *mp++ = 'b';
    m_handle = gzopen(p, fm);
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
    if (m_mode == WRITE) return -1;
    if (!dest || !m_handle)
        return -1;

    return gzread((gzFile)m_handle, dest, nr);
}


FBTsize ftGzStream::write(const void* src, FBTsize nr)
{
    if (m_mode == READ) return -1;
    if (!src || !m_handle) return -1;
    return gzwrite((gzFile)m_handle, src, nr);
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


FBTsize ftGzStream::writef(const char* fmt, ...)
{
    static char tmp[1024];

    va_list lst;
    va_start(lst, fmt);
    int size = ftp_printf(tmp, 1024, fmt, lst);
    va_end(lst);

    if (size > 0)
    {
        tmp[size] = 0;
        return write(tmp, size);
    }
    return -1;
}
#endif
