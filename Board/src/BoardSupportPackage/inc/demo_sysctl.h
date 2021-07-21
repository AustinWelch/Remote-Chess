
#ifndef __DEMO_SYSCTL_H__
#define __DEMO_SYSCTL_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

// #define DelayMs(ulClockMS) {SysCtlDelay((ulClockMS/2)*(CS_getMCLK() / (3 * 1000)));}
extern void DelayMs (uint32_t ulClockMS);
//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void SysCtlDelay(uint32_t ui32Count);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __DRIVERLIB_SYSCTL_H__
