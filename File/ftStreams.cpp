/*
-------------------------------------------------------------------------------
    Copyright (c) 2010 Charlie C & Erwin Coumans.

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
#define ftIN_SOURCE
#include "ftStreams.h"
#include "ftPlatformHeaders.h"


#if ftUSE_GZ_FILE == 1
#include "zlib.h"
#include "zconf.h"
#endif


ftFileStream::ftFileStream() 
	:    m_file(), m_handle(0), m_mode(0), m_size(0)
{
}


ftFileStream::~ftFileStream()
{
	close();
}



void ftFileStream::open(const char* p, ftStream::StreamMode mode)
{
	if (m_handle != 0 && m_file != p)
		fclose((FILE *)m_handle);

	char fm[3] = {0, 0, 0};
	char* mp = &fm[0];
	if (mode & ftStream::SM_READ)
		*mp++ = 'r';
	else if (mode & ftStream::SM_WRITE)
		*mp++ = 'w';
	*mp++ = 'b';
	fm[2] = 0;

	m_file = p;
	m_handle = fopen(m_file.c_str(), fm);

	if (m_handle && (mode & ftStream::SM_READ))
	{
		FILE *fp = (FILE*)m_handle;

		position();
		fseek(fp, 0, SEEK_END);
		m_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
	}


}


void ftFileStream::close(void)
{
	if (m_handle != 0)
	{
		fclose((FILE*)m_handle);
		m_handle = 0;
	}

	m_file.clear();
}


bool ftFileStream::eof(void) const
{
	if (!m_handle)
		return true;
	return feof((FILE*)m_handle) != 0;
}

FBTsize ftFileStream::position(void) const
{
	return ftell((FILE*)m_handle);
}


FBTsize ftFileStream::size(void) const
{
	return m_size;
}

FBTsize ftFileStream::seek(FBTint32 off, FBTint32 way)
{
	if (!m_handle)
		return 0;

	return fseek((FILE*)m_handle, off, way);
}


FBTsize ftFileStream::read(void* dest, FBTsize nr) const
{
	if (m_mode == ftStream::SM_WRITE) 
		return -1;

	if (!dest || !m_handle) 
		return -1;

	return fread(dest, 1, nr, (FILE *)m_handle);
}



FBTsize ftFileStream::write(const void* src, FBTsize nr)
{
	if (m_mode == ftStream::SM_READ) return -1;
	if (!src || !m_handle) return -1;

	return fwrite(src, 1, nr, (FILE*)m_handle);
}

void ftFileStream::write(ftMemoryStream &ms) const
{
	FILE *fp = (FILE*)m_handle;

	int oldPos = ftell(fp);

	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	ms.reserve(len + 1);
	ms.m_size = read(ms.m_buffer, len);

	fseek(fp, oldPos, SEEK_SET);
}


FBTsize ftFileStream::writef(const char* fmt, ...)
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


#if ftUSE_GZ_FILE == 1


ftGzStream::ftGzStream() 
	:    m_file(), m_handle(0), m_mode(0)
{
}


ftGzStream::~ftGzStream()
{
	close();
}


void ftGzStream::open(const char* p, ftStream::StreamMode mode)
{
	if (m_handle != 0 && m_file != p)
		gzclose(m_handle);

	char fm[3] = {0, 0, 0};
	char* mp = &fm[0];
	if (mode & ftStream::SM_READ)
		*mp++ = 'r';
	else if (mode & ftStream::SM_WRITE)
		*mp++ = 'w';
	*mp++ = 'b';
	fm[2] = 0;

	m_file = p;
	m_handle = gzopen(m_file.c_str(), fm);
}


void ftGzStream::close(void)
{
	if (m_handle != 0)
	{
		gzclose(m_handle);
		m_handle = 0;
	}

	m_file.clear();
}



FBTsize ftGzStream::read(void* dest, FBTsize nr) const
{
	if (m_mode == ftStream::SM_WRITE) return -1;
	if (!dest || !m_handle) 
		return -1;

	return gzread(m_handle, dest, nr);
}


FBTsize ftGzStream::write(const void* src, FBTsize nr)
{
	if (m_mode == ftStream::SM_READ) return -1;
	if (!src || !m_handle) return -1;
	return gzwrite(m_handle, src, nr);
}


bool ftGzStream::eof(void) const
{
	if (!m_handle)
		return true;
	return gzeof(m_handle) != 0;
}

FBTsize ftGzStream::position(void) const
{
	return gztell(m_handle);
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




ftMemoryStream::ftMemoryStream()
	:   m_buffer(0), m_pos(0), m_size(0), m_capacity(0), m_mode(0)
{
}


void ftMemoryStream::open(ftStream::StreamMode mode)
{
	m_mode = mode;
}

void ftMemoryStream::open(const char* path, ftStream::StreamMode mode)
{
	ftFileStream fs;
	fs.open(path, ftStream::SM_READ);

	if (fs.isOpen()) 
		open(fs, mode);
}


void ftMemoryStream::open(const ftFileStream& fs, ftStream::StreamMode mode)
{
	if (fs.isOpen())
	{
		fs.write(*this);
		m_mode = mode;
	}
}


void ftMemoryStream::open(const void* buffer, FBTsize size, ftStream::StreamMode mode, bool compressed)
{
	if (buffer && size > 0 && size != ftNPOS)
	{
		m_mode = mode;
		m_pos  = 0;


		if (!compressed)
		{
			m_size = size;
			reserve(m_size);
			ftMemcpy(m_buffer, buffer, m_size);

		} else
		{
#if ftUSE_GZ_FILE == 1
			bool result = gzipInflate((char*)buffer,size);
#endif
		}

	}
}

#if ftUSE_GZ_FILE == 1
// this method was adapted from this snippet:
// http://windrealm.org/tutorials/decompress-gzip-stream.php
bool ftMemoryStream::gzipInflate( char* inBuf, int inSize) {
  if ( inSize == 0 ) {
	   m_buffer = inBuf ;
    return true ;
  }

  if (m_buffer)
	  delete [] m_buffer;


  m_size = inSize ;
  m_buffer = (char*) calloc( sizeof(char), m_size );

  z_stream strm;
  strm.next_in = (Bytef *) inBuf;
  strm.avail_in = inSize ;
  strm.total_out = 0;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;

  bool done = false ;

  if (inflateInit2(&strm, (16+MAX_WBITS)) != Z_OK) {
    free( m_buffer );
    return false;
  }

  while (!done) {
    // If our output buffer is too small
    if (strm.total_out >= m_size ) {
      // Increase size of output buffer
      char* uncomp2 = (char*) calloc( sizeof(char), m_size + (inSize / 2) );
      memcpy( uncomp2, m_buffer, m_size );
      m_size += inSize / 2 ;
      free( m_buffer );
      m_buffer = uncomp2 ;
    }

    strm.next_out = (Bytef *) (m_buffer + strm.total_out);
    strm.avail_out = m_size - strm.total_out;

    // Inflate another chunk.
    int err = inflate (&strm, Z_SYNC_FLUSH);
    if (err == Z_STREAM_END) done = true;
    else if (err != Z_OK)  {
      break;
    }
  }

  if (inflateEnd (&strm) != Z_OK) {
    free( m_buffer );
    return false;
  }

  return done ;
}
#endif

ftMemoryStream::~ftMemoryStream()
{
	if (m_buffer != 0 )
	{
		delete []m_buffer;
	}
	m_buffer = 0;
	m_size = m_pos = 0;
	m_capacity = 0;
}



void ftMemoryStream::clear(void)
{
	m_size = m_pos = 0;
	if (m_buffer)
		m_buffer[0] = 0;
}


FBTsize ftMemoryStream::seek(FBTint32 off, FBTint32 way)
{
	if (way == SEEK_SET)
		m_pos = ftClamp<FBTsize>(off, 0, m_size);
	else if (way == SEEK_CUR)
		m_pos = ftClamp<FBTsize>(m_pos + off, 0, m_size);
	else if (way == SEEK_END)
		m_pos = m_size;
	return m_pos;
}



FBTsize ftMemoryStream::read(void* dest, FBTsize nr) const
{
	if (m_mode == ftStream::SM_WRITE) return -1;
	if (m_pos > m_size) return 0;
	if (!dest || !m_buffer) return 0;

	if ((m_size - m_pos) < nr) nr = m_size - m_pos;

	char* cp = (char*)dest;
	char* op = (char*)&m_buffer[m_pos];
	ftMemcpy(cp, op, nr);
	m_pos += nr;
	return nr;
}



FBTsize ftMemoryStream::write(const void* src, FBTsize nr)
{
	if (m_mode == ftStream::SM_READ || !src)
		return -1;

	if (m_pos > m_size) return 0;

	if (m_buffer == 0)
		reserve(m_pos + (nr));
	else if (m_pos + nr > m_capacity)
		reserve(m_pos + (nr > 65535 ? nr : nr + 65535));

	char* cp = &m_buffer[m_pos];
	ftMemcpy(cp, (char*)src, nr);

	m_pos   += nr;
	m_size  += nr;
	return nr;
}



FBTsize ftMemoryStream::writef(const char* fmt, ...)
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
void ftMemoryStream::reserve(FBTsize nr)
{
	if (m_capacity < nr)
	{
		char* buf = new char[nr + 1];
		if (m_buffer != 0)
		{
			ftMemcpy(buf, m_buffer, m_size);
			delete [] m_buffer;
		}

		m_buffer = buf;
		m_buffer[m_size] = 0;
		m_capacity = nr;
	}
}
