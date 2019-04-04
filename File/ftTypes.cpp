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
#include "ftTypes.h"


#define ftIN_SOURCE
#include "ftPlatformHeaders.h"


// ----------------------------------------------------------------------------
// Debug Utilities


#ifdef ftDEBUG
bool ftDebugger::isDebugger(void)
{
#if ftCOMPILER == ftCOMPILER_MSVC
	return IsDebuggerPresent() != 0;
#else
	return false;
#endif
}

void ftDebugger::breakProcess(void)
{
#if ftCOMPILER == ftCOMPILER_MSVC
	_asm int 3;
#else
	asm("int $3");
#endif
}

#else//ftDEBUG


bool ftDebugger::isDebugger(void)
{
	return false;
}

void ftDebugger::breakProcess(void)
{
}

#endif//ftDEBUG


#define ftDEBUG_BUF_SIZE (1024)
ftDebugger::Reporter ftDebugger::m_report = {0, 0};


void ftDebugger::setReportHook(Reporter& hook)
{
	m_report.m_client = hook.m_client;
	m_report.m_hook   = hook.m_hook;
}


void ftDebugger::report(const char* fmt, ...)
{
	char ReportBuf[ftDEBUG_BUF_SIZE+1];

	va_list lst;
	va_start(lst, fmt);
	int size = ftp_printf(ReportBuf, ftDEBUG_BUF_SIZE, fmt, lst);
	va_end(lst);

	if (size < 0)
	{
		ReportBuf[ftDEBUG_BUF_SIZE] = 0;
		size = ftDEBUG_BUF_SIZE;
	}

	if (size > 0)
	{
		ReportBuf[size] = 0;

		if (m_report.m_hook)
		{
#if ftCOMPILER == ftCOMPILER_MSVC
			if (IsDebuggerPresent())
				OutputDebugString(ReportBuf);

#endif
			m_report.m_hook(m_report.m_client, ReportBuf);
		}
		else
		{

#if ftCOMPILER == ftCOMPILER_MSVC
			if (IsDebuggerPresent())
				OutputDebugString(ReportBuf);
			else
#endif
				fprintf(stderr, "%s", ReportBuf);
		}
	}

}


void ftDebugger::reportIDE(const char* src, long line, const char* fmt, ...)
{
	static char ReportBuf[ftDEBUG_BUF_SIZE+1];

	va_list lst;
	va_start(lst, fmt);


	int size = ftp_printf(ReportBuf, ftDEBUG_BUF_SIZE, fmt, lst);
	va_end(lst);

	if (size < 0)
	{
		ReportBuf[ftDEBUG_BUF_SIZE] = 0;
		size = ftDEBUG_BUF_SIZE;
	}

	if (size > 0)
	{
		ReportBuf[size] = 0;
#if ftCOMPILER == ftCOMPILER_MSVC
		report("%s(%i): warning: %s", src, line, ReportBuf);
#else
		report("%s:%i: warning: %s", src, line, ReportBuf);
#endif
	}
}


void ftDebugger::errorIDE(const char* src, long line, const char* fmt, ...)
{
	static char ReportBuf[ftDEBUG_BUF_SIZE+1];

	va_list lst;
	va_start(lst, fmt);


	int size = ftp_printf(ReportBuf, ftDEBUG_BUF_SIZE, fmt, lst);
	va_end(lst);

	if (size < 0)
	{
		ReportBuf[ftDEBUG_BUF_SIZE] = 0;
		size = ftDEBUG_BUF_SIZE;
	}

	if (size > 0)
	{
		ReportBuf[size] = 0;
#if ftCOMPILER == ftCOMPILER_MSVC
		report("%s(%i): error: %s", src, line, ReportBuf);
#else
		report("%s:%i: error: %s", src, line, ReportBuf);
#endif
	}
}

ftPRIM_TYPE ftGetPrimType(FBTuint32 typeKey)
{
	static FBTuint32 charT    = ftCharHashKey("char").hash();
	static FBTuint32 ucharT   = ftCharHashKey("uchar").hash();
	static FBTuint32 shortT   = ftCharHashKey("short").hash();
	static FBTuint32 ushortT  = ftCharHashKey("ushort").hash();
	static FBTuint32 intT     = ftCharHashKey("int").hash();
	static FBTuint32 longT    = ftCharHashKey("long").hash();
	static FBTuint32 ulongT   = ftCharHashKey("ulong").hash();
	static FBTuint32 floatT   = ftCharHashKey("float").hash();
	static FBTuint32 doubleT  = ftCharHashKey("double").hash();
	static FBTuint32 voidT    = ftCharHashKey("void").hash();

	if (typeKey == charT)	return ftPRIM_CHAR;
	if (typeKey == ucharT)	return ftPRIM_UCHAR;
	if (typeKey == shortT)	return ftPRIM_SHORT;
	if (typeKey == ushortT)	return ftPRIM_USHORT;
	if (typeKey == intT)	return ftPRIM_INT;
	if (typeKey == longT)	return ftPRIM_LONG;
	if (typeKey == ulongT)	return ftPRIM_ULONG;
	if (typeKey == floatT)	return ftPRIM_FLOAT;
	if (typeKey == doubleT)	return ftPRIM_DOUBLE;
	if (typeKey == voidT)	return ftPRIM_VOID;

	return ftPRIM_UNKNOWN;
}



