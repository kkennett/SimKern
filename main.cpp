#include <stdio.h>
#include <windows.h>

typedef struct _SKThread SKThread;
struct _SKThread
{
    HANDLE      mhThread;
    CONTEXT     mContext;
};

typedef struct _SKCpu SKCpu;
struct _SKCpu
{
    SKSystem * const    mpSystem;
    HANDLE              mhKernelThread;
    SKThread *          mpCurrentThread;
};

typedef struct _SKSystem SKSystem;
struct _SKSystem
{
    UINT_PTR    mCpuCount;
    SKCpu      *mpCpu;
};

SKSystem theSystem;


bool 
SKInitCpu(
    UINT_PTR aCpuIndex
)
{
    
}

void Startup(void)
{
    UINT_PTR ix;

    theSystem.mCpuCount = GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);
    if (theSystem.mCpuCount > 64)
        theSystem.mCpuCount = 64;

    theSystem.mpCpu = (SKCpu *)malloc(theSystem.mCpuCount * sizeof(SKCpu));
    if (NULL == theSystem.mpCpu)
    {
        printf("malloc failed for cpu array\n");
        ExitProcess(-1);
    }

    for (ix = 0; ix < theSystem.mpCpu)
    {
        SKInitCpu(ix);
    }
}

void Run(void)
{

}

int main(int argc, char** argv)
{
    Startup();
    Run();
    return 0;
}