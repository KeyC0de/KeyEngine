#include "winner.h"
#include <dbghelp.h>
#include <strsafe.h>
#include "dumpling.h"

#pragma comment( lib, "dbghelp.lib" )


bool generateDump( EXCEPTION_POINTERS *pExceptionPointers,
	WCHAR *szFileName )
{
	BOOL bMiniDumpSuccessful;
	WCHAR szPath[MAX_PATH];
	const wchar_t *szAppName = L"AppName";
	const wchar_t *szVersion = L"v1.0";
	DWORD dwBufferSize = MAX_PATH;
	HANDLE hDumpFile;
	SYSTEMTIME stLocalTime;
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;

	GetLocalTime( &stLocalTime );
	GetTempPathW( dwBufferSize, szPath );
	StringCchPrintfW( szFileName, MAX_PATH, L"%s%s", szPath, szAppName );
	CreateDirectoryW( szFileName, nullptr );
	StringCchPrintfW( szFileName,
		MAX_PATH,
		L"%s%s/%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",
		szPath,
		szAppName,
		szVersion,
		stLocalTime.wYear,
		stLocalTime.wMonth,
		stLocalTime.wDay,
		stLocalTime.wHour,
		stLocalTime.wMinute,
		stLocalTime.wSecond,
		GetCurrentProcessId(),
		GetCurrentThreadId() );
	hDumpFile = CreateFileW( szFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		nullptr,
		CREATE_ALWAYS,
		0u,
		nullptr );

	ExpParam.ThreadId = GetCurrentThreadId();
	ExpParam.ExceptionPointers = pExceptionPointers;
	ExpParam.ClientPointers = TRUE;

	bMiniDumpSuccessful = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithDataSegs, &ExpParam, nullptr, nullptr );

	return EXCEPTION_EXECUTE_HANDLER && bMiniDumpSuccessful;
}