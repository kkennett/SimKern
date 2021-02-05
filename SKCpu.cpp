#include "SimKern.h"

void SKCpu_OnStartup(SKCpu *apThisCpu)
{
    //
    // main cpu kernel thread startup - 
    // idle thread is current but it is suspended
    //

}

void SKCpu_OnShutdown(SKCpu *apThisCpu)
{
    //
    // main cpu loop is exiting in kernel thread
    //

}

void SKCpu_OnInterrupt(SKCpu *apThisCpu)
{
    //
    // entry to kernel that is not from the current thread, so an IRQ
    // change mpCurrentThread if you want some other thread to run
    // when the function exits
    //

}

void SKCpu_OnSystemCall(SKCpu *apThisCpu)
{
    //
    // apThisCpu->mpCurrentThread is making a system call
    // change mpCurrentThread if you want some other thread to run
    // when the function exits
    //

}

void SKCpu_OnRecvIci(SKCpu *apThisCpu, SKCpu *apSenderCpu, UINT_PTR aCode)
{
    //
    // we received an ICI from the sender Cpu with the corresponding code
    // change mpCurrentThread if you want some other thread to run
    // when the function exits
    //

}

