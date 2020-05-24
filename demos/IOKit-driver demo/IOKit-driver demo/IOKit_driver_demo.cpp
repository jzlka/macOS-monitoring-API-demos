//
//  IOKit_driver_demo.cpp
//  IOKit-driver demo
//
//  Created by Jozef on 24/05/2019.
//  Copyright © 20200 Jozef Zuzelka. All rights reserved.
//
// sudo kextload "IOKit-driver demo.kext"
// grep IOKitDemo /var/log/system.log

#include <IOKit/IOLib.h>
#define super IOService

OSDefineMetClassAndStructors(com_jzlka_driver_IOKitDemo, IOService)

bool com_jzlka_driver_IOKitDemo::init(OSDictionary *dict)
{
    bool res = super::init(dict);
    IOLog("IOKitDemo::init\n");
    return res;
}

IOService* com_jzlka_driver_IOKitDemo::probe(IOService *provider, SInt32 *score)
{
    IOService *res = super::probe(provider, score);
    IOLog("IOKitDemo::probe\n");
    return res;
}

bool com_jzlka_driver_IOKitDemo::start(IOService *provider)
{
    bool res = super::start(provider);
    IOLog("IOKitDemo::start\n");
    return res;
}
