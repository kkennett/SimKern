#include "SimKern.h"

HANDLE      theWin32ExitSignal;
SKSystem    theSystem;

DWORD WINAPI SKKernelThread(void *apParam)
{
    SKCpu * pThisCpu = (SKCpu *)apParam;
    DWORD   waitResult;
    HANDLE  waitHandles[2];

    SKCpu_OnStartup(pThisCpu);

    if (FALSE == SetEvent(pThisCpu->mhWin32ActionEvent))
    {
        printf("Could not set start event for CPU %d\n", pThisCpu->mCpuIndex);
        ExitProcess(__LINE__);
    }

    // 
    // idle thread is current thread, suspsended
    // 
    if (((DWORD)-1) == ResumeThread(pThisCpu->mpIdleThread->mhWin32Thread))
    {
        printf("Could not resume idle thread at startup of CPU %d\n", pThisCpu->mCpuIndex);
        ExitProcess(__LINE__);
    }

    waitHandles[0] = pThisCpu->mhWin32ActionEvent;
    waitHandles[1] = pThisCpu->mhWin32SysCallEvent;
    printf("CPU %d IDLE\n", pThisCpu->mCpuIndex);
    do
    {
        waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

        if (((DWORD)-1) == SuspendThread(pThisCpu->mpCurrentThread->mhWin32Thread))
        {
            printf("Could not suspend current thread on CPU %d\n", pThisCpu->mCpuIndex);
            ExitProcess(__LINE__);
        }
        if (pThisCpu->mpCurrentThread == pThisCpu->mpIdleThread)
        {
            printf("CPU %d EXIT IDLE\n", pThisCpu->mCpuIndex);
            ExitProcess(__LINE__);
        }

        if (waitResult == WAIT_OBJECT_0)
        {
            //
            // logical interrupt not related to currently executing thread (IPI or IRQ)
            //
            SKCpu_OnInterrupt(pThisCpu);
        }
        else if (waitResult == WAIT_OBJECT_0 + 1)
        {
            // System Call from current thread
            if (pThisCpu->mpCurrentThread == pThisCpu->mpIdleThread)
            {
                printf("Idle thread did system call on CPU %d\n", pThisCpu->mCpuIndex);
                ExitProcess(__LINE__);
            }

            //
            // take activity based on reason for current thread entering 'kernel'
            //
            SKCpu_OnSystemCall(pThisCpu);
        }
        else
        {
            printf("Failed to wait for action event on CPU %d\n", pThisCpu->mCpuIndex);
            ExitProcess(__LINE__);
        }

        if (pThisCpu->mpCurrentThread == pThisCpu->mpIdleThread)
        {
            printf("CPU %d ENTER IDLE\n", pThisCpu->mCpuIndex);
        }

        if (((DWORD)-1) == ResumeThread(pThisCpu->mpCurrentThread->mhWin32Thread))
        {
            printf("Could not resume current thread of CPU %d\n", pThisCpu->mCpuIndex);
            ExitProcess(__LINE__);
        }

    } while (true);

    SKCpu_OnShutdown(pThisCpu);

    return 0;
}

DWORD WINAPI SKIdleThread(void *apParam)
{
    SKThread *pThisThread = (SKThread *)apParam;
    do
    {
        Sleep(1000);
    } while (true);
    return 0;
}

void 
SKInitCpu(
    SKSystem *  apSystem,
    DWORD       aCpuIndex
)
{
    SKThread *  pIdleThread;
    SKCpu *     pCpu;
    
    pCpu = &apSystem->mpCpus[aCpuIndex];
    pCpu->mpSystem = apSystem;
    pCpu->mCpuIndex = aCpuIndex;
    pCpu->mpIdleThread = pIdleThread = (SKThread *)malloc(sizeof(SKThread));
    if (NULL == pIdleThread)
    {
        printf("Could not allocate idle thread for CPU %d\n", aCpuIndex);
        ExitProcess(__LINE__);
    }

    pCpu->mhWin32ActionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == pCpu->mhWin32ActionEvent)
    {
        printf("Could not create action event for CPU %d\n", aCpuIndex);
        ExitProcess(__LINE__);
    }

    pCpu->mhWin32SysCallEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == pCpu->mhWin32SysCallEvent)
    {
        printf("Could not create syscall event for CPU %d\n", aCpuIndex);
        ExitProcess(__LINE__);
    }

    pIdleThread->mhWin32Thread = CreateThread(
        NULL, 
        0, 
        SKIdleThread, 
        pIdleThread, 
        CREATE_SUSPENDED, 
        &pIdleThread->mWin32ThreadId);
    if (NULL == pIdleThread->mhWin32Thread)
    {
        printf("Could not create suspended idle thread for CPU %d\n", aCpuIndex);
        ExitProcess(__LINE__);
    }

    if (0 == SetThreadAffinityMask(pIdleThread->mhWin32Thread, ((DWORD_PTR)1) << aCpuIndex))
    {
        printf("Could not set affinity of idle thread for CPU %d\n", aCpuIndex);
        ExitProcess(__LINE__);
    }

    pCpu->mhWin32KernelThread = CreateThread(
        NULL,
        0,
        SKKernelThread,
        pCpu,
        CREATE_SUSPENDED,
        &pCpu->mWin32KernelThreadId);
    if (NULL == pCpu->mhWin32KernelThread)
    {
        printf("Could not create suspended idle thread for CPU %d\n", aCpuIndex);
        ExitProcess(__LINE__);
    }

    if (0 == SetThreadAffinityMask(pCpu->mhWin32KernelThread, ((DWORD_PTR)1) << aCpuIndex))
    {
        printf("Could not set affinity of kernel thread for CPU %d\n", aCpuIndex);
        ExitProcess(__LINE__);
    }

    if (0 == SetThreadPriority(pCpu->mhWin32KernelThread, THREAD_PRIORITY_HIGHEST))
    {
        printf("Could not set priority of kernel thread for CPU %d\n", aCpuIndex);
        ExitProcess(__LINE__);
    }

    pCpu->mpCurrentThread = pCpu->mpIdleThread;
    pIdleThread->mpCurrentCpu = pCpu;

    if (((DWORD)-1) == ResumeThread(pCpu->mhWin32KernelThread))
    {
        printf("Could not resume kernel thread for CPU %d\n", aCpuIndex);
        ExitProcess(__LINE__);
    }
    if (WAIT_OBJECT_0 != WaitForSingleObject(pCpu->mhWin32ActionEvent, INFINITE))
    {
        printf("Waiting for kernel thread to start failed for CPU %d\n", aCpuIndex);
        ExitProcess(__LINE__);
    }
}

void Startup(void)
{
    UINT_PTR ix;

    theWin32ExitSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == theWin32ExitSignal)
    {
        printf("failed to create exit signal\n");
        ExitProcess(__LINE__);
    }

    theSystem.mCpuCount = GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);
    if (theSystem.mCpuCount > 64)
        theSystem.mCpuCount = 64;

    theSystem.mpCpus = (SKCpu *)malloc(theSystem.mCpuCount * sizeof(SKCpu));
    if (NULL == theSystem.mpCpus)
    {
        printf("malloc failed for cpu array\n");
        ExitProcess(__LINE__);
    }

    for (ix = 0; ix < theSystem.mCpuCount; ix++)
    {
        SKInitCpu(&theSystem, (DWORD)ix);
    }
}

void Run(void)
{
    WaitForSingleObject(theWin32ExitSignal, INFINITE);
}

void Shutdown(void)
{

}

int main(int argc, char** argv)
{
    Startup();
    Run();
    Shutdown();
    return 0;
}