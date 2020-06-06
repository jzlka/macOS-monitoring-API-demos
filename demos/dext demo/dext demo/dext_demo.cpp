//
//  dext_demo.cpp
//  dext demo
//
//  Created by Jozef on 06/06/2020.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//
// Sources:
// https://github.com/knightsc/USBApp/

#include <os/log.h>
#include <DriverKit/IOUserServer.h>
#include <DriverKit/IOLib.h>

#include <USBDriverKit/USBDriverKit.h>
//#include <USBDriverKit/USBDriverKit.h>
//#include <USBDriverKit/IOUSBHostPipe.h>

#include "dext_demo.h"


#define __Require(assertion, exceptionLabel)                    \
do {                                                            \
    if ( __builtin_expect(!(assertion), 0) ) {                  \
        goto exceptionLabel;                                    \
    }                                                           \
} while (0)

#define __Require_Action(assertion, exceptionLabel, action)     \
do {                                                            \
    if ( __builtin_expect(!(assertion), 0) ) {                  \
        {                                                       \
            action;                                             \
        }                                                       \
        goto exceptionLabel;                                    \
    }                                                           \
} while (0)

static const uint32_t kMyEndpointAddress = 1;

struct dext_demo_IVars
{
    IOUSBHostInterface       *interface;
    IOUSBHostPipe            *inPipe;
    OSAction                 *ioCompleteCallback;
    IOBufferMemoryDescriptor *inData;
    uint16_t                  maxPacketSize;
};

bool dext_demo::init()
{
    bool result = false;

    os_log(OS_LOG_DEFAULT, "%s", __FUNCTION__);

    result = super::init();
    __Require(true == result, Exit);

    ivars = /*(IOService_IVars *)*/ IONewZero(dext_demo_IVars, 1);
    __Require_Action(NULL != ivars, Exit, result = false);

Exit:
    return result;
}

kern_return_t IMPL(dext_demo, Start)
{
    kern_return_t                    ret;
    IOUSBStandardEndpointDescriptors descriptors;

    os_log(OS_LOG_DEFAULT, "%s", __FUNCTION__);
    dext_demo_IVars *ivars_local = (dext_demo_IVars *) ivars;

    ret = Start(provider, SUPERDISPATCH);
    __Require(kIOReturnSuccess == ret, Exit);
    os_log(OS_LOG_DEFAULT, "Hello World");

//    ivars_local->interface = OSDynamicCast(IOUSBHostInterface, provider);
//    __Require_Action(NULL != ivars_local->interface, Exit, ret = kIOReturnNoDevice);
//
//    ret = ivars_local->interface->Open(this, 0, NULL);
//    __Require(kIOReturnSuccess == ret, Exit);
//
//    ret = ivars_local->interface->CopyPipe(kMyEndpointAddress, &ivars_local->inPipe);
//    __Require(kIOReturnSuccess == ret, Exit);
//
//    ret = ivars_local->interface->CreateIOBuffer(kIOMemoryDirectionIn,
//                                           ivars_local->maxPacketSize,
//                                           &ivars_local->inData);
//    __Require(kIOReturnSuccess == ret, Exit);

//    ret = OSAction::Create(this,
//                           MyUserUSBInterfaceDriver_ReadComplete_ID,
//                           IOUSBHostPipe_CompleteAsyncIO_ID,
//                           0,
//                           &ivars_local->ioCompleteCallback);
//    __Require(kIOReturnSuccess == ret, Exit);
//
//    ret = ivars_local->inPipe->AsyncIO(ivars_local->inData,
//                                 ivars_local->maxPacketSize,
//                                 ivars_local->ioCompleteCallback,
//                                 0);
//    __Require(kIOReturnSuccess == ret, Exit);

    // WWDC slides don't show the full function https://developer.apple.com/videos/play/wwdc2019/702/?time=1418
    // i.e. this is still unfinished

Exit:
    return ret;
}

kern_return_t IMPL(dext_demo, Stop)
{
    kern_return_t ret = kIOReturnSuccess;

    os_log(OS_LOG_DEFAULT, "%s", __FUNCTION__);

    return ret;
}

void dext_demo::free()
{
    os_log(OS_LOG_DEFAULT, "%s", __FUNCTION__);
}

void IMPL(dext_demo, ReadComplete)
{
    os_log(OS_LOG_DEFAULT, "%s", __FUNCTION__);

    os_log(OS_LOG_DEFAULT, "Spinning forever\n");
    bool loop = true;
    while (true == loop) { }

    os_log(OS_LOG_DEFAULT, "Goodbye\n");
    *(volatile uint32_t *)0 = 0xdeadbeef;
}
