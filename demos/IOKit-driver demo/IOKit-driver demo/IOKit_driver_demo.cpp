//
//  IOKit_driver_demo.cpp
//  IOKit-driver demo
//
//  Created by Jozef on 24/05/2019.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//
// sudo kextload "IOKit-driver demo.kext"
// grep IOKitDemo /var/log/system.log

#include <IOKit/IOLib.h>
#include "IOKit_driver_demo.hpp"
//#define super IOService

// This required macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.
OSDefineMetaClassAndStructors(com_jzlka_driver_IOKit_demo, IOService)

// Define the driver's superclass.
#define super IOService

bool com_jzlka_driver_IOKit_demo::init(OSDictionary *dict)
{
    bool result = super::init(dict);
    IOLog("Initializing\n");
    return result;
}

void com_jzlka_driver_IOKit_demo::free(void)
{
    IOLog("Freeing\n");
    super::free();
}

IOService *com_jzlka_driver_IOKit_demo::probe(IOService *provider,
    SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    IOLog("Probing\n");
    return result;
}

bool com_jzlka_driver_IOKit_demo::start(IOService *provider)
{
    bool result = super::start(provider);
    IOLog("Starting\n");
    return result;
}

void com_jzlka_driver_IOKit_demo::stop(IOService *provider)
{
    IOLog("Stopping\n");
    super::stop(provider);
}
