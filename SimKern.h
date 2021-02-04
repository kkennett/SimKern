#ifndef __SIMKERN_H
#define __SIMKERN_H

#include <stdio.h>
#include <windows.h>

typedef struct _SKSystem SKSystem;
typedef struct _SKCpu SKCpu;
typedef struct _SKThread SKThread;

struct _SKThread
{
    HANDLE      mhWin32Thread;
    DWORD       mWin32ThreadId;
    SKCpu *     mpCurrentCpu;
};

struct _SKCpu
{
    SKSystem *  mpSystem;
    DWORD       mCpuIndex;

    HANDLE      mhWin32ActionEvent;
    HANDLE      mhWin32SysCallEvent;

    HANDLE      mhWin32KernelThread;
    DWORD       mWin32KernelThreadId;

    SKThread *  mpCurrentThread;

    SKThread *  mpIdleThread;
};

typedef struct _SKSpinLock SKSpinLock;
struct _SKSpinLock
{
    CRITICAL_SECTION    mWin32Sec;
};

struct _SKSystem
{
    UINT_PTR    mCpuCount;
    SKCpu      *mpCpus;
};

extern HANDLE   theWin32ExitSignal;
extern SKSystem theSystem;

void SKSpinLock_Init(SKSpinLock *apLock);
void SKSpinLock_Lock(SKSpinLock *apLock);
void SKSpinLock_Unlock(SKSpinLock *apLock);

void SKCpu_OnStartup(SKCpu *apThisCpu);
void SKCpu_OnShutdown(SKCpu *apThisCpu);
void SKCpu_OnInterrupt(SKCpu *apThisCpu);
void SKCpu_OnSystemCall(SKCpu *apThisCpu);

#endif // __SIMKERN_H