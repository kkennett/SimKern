#ifndef __SIMKERN_H
#define __SIMKERN_H

#include <stdio.h>
#include <windows.h>

typedef struct _SKSystem SKSystem;
typedef struct _SKCpu SKCpu;
typedef struct _SKThread SKThread;

typedef struct _SKICI SKICI;
struct _SKICI
{
    SKCpu *             mpSenderCpu;
    UINT_PTR            mCode;
    volatile SKICI *    mpNext;
};

struct _SKThread
{
    HANDLE          mhWin32Thread;
    DWORD           mWin32ThreadId;
    SKCpu *         mpCurrentCpu;
    LARGE_INTEGER   mRunTime;
};

struct _SKCpu
{
    SKSystem *          mpSystem;
    DWORD               mCpuIndex;

    HANDLE              mhWin32IciEvent;
    HANDLE              mhWin32IrqEvent;
    HANDLE              mhWin32SysCallEvent;

    HANDLE              mhWin32KernelThread;
    DWORD               mWin32KernelThreadId;

    SKThread *          mpCurrentThread;

    SKThread *          mpIdleThread;

    LARGE_INTEGER       mSchedTimeout;
    LARGE_INTEGER       mTimeNow;

    volatile SKICI *    mpPendingIcis;
};

typedef struct _SKSpinLock SKSpinLock;
struct _SKSpinLock
{
    CRITICAL_SECTION    mWin32Sec;
};

struct _SKSystem
{
    UINT_PTR        mCpuCount;
    SKCpu      *    mpCpus;
    LARGE_INTEGER   mPerfFreq;
};

extern HANDLE   theWin32ExitSignal;
extern SKSystem theSystem;

void SKSpinLock_Init(SKSpinLock *apLock);
void SKSpinLock_Lock(SKSpinLock *apLock);
void SKSpinLock_Unlock(SKSpinLock *apLock);

void SKKernel_SendIci(SKCpu *apThisCpu, DWORD aTargetCpu, UINT_PTR aCode);
void SKKernel_MsToCpuTime(DWORD aMilliseconds, LARGE_INTEGER *apCpuTime);
DWORD SKKernel_CpuTimeToMs(LARGE_INTEGER *apCpuTime);

void SKCpu_OnStartup(SKCpu *apThisCpu);
void SKCpu_OnShutdown(SKCpu *apThisCpu);
void SKCpu_OnRecvIci(SKCpu *apThisCpu, SKCpu *apSenderCpu, UINT_PTR aCode);
void SKCpu_OnIrqInterrupt(SKCpu *apThisCpu);
void SKCpu_OnSchedTimerExpiry(SKCpu *apThisCpu);
void SKCpu_OnSystemCall(SKCpu *apThisCpu);

#endif // __SIMKERN_H