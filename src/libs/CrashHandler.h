/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2011 Arctic Server Team
 * See COPYING for license details.
 */

#ifndef _CRASH_HANDLER_H
#define _CRASH_HANDLER_H

bool HookCrashReporter(bool logon);
void OutputCrashLogLine(const char* format, ...);

#ifdef WIN32

//#include <Windows.h>
#include "Common.h"
#include <DbgHelp.h>
#include "StackWalker.h"
#include "CircularQueue.h"

extern CircularQueue<uint32, 30> last_spells;

class SERVER_DECL CStackWalker : public StackWalker
{
public:
	void OnOutput(LPCSTR szText);
	void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName);
	void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion);
	void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry);
	void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr);
};

void StartCrashHandler();
void OnCrash(bool Terminate);

typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
int __cdecl HandleCrash(PEXCEPTION_POINTERS pExceptPtrs);

#define THREAD_TRY_EXECUTION __try 
#define THREAD_HANDLE_CRASH  __except(HandleCrash(GetExceptionInformation())) {}

#define THREAD_TRY_EXECUTION2 __try {
#define THREAD_HANDLE_CRASH2  } __except(HandleCrash(GetExceptionInformation())) {}

#else

// We dont wanna confuse nix ;p
#define THREAD_TRY_EXECUTION 
#define THREAD_HANDLE_CRASH 

#define THREAD_TRY_EXECUTION2 ;
#define THREAD_HANDLE_CRASH2 ;

#endif

#endif

