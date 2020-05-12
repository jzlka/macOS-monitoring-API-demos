//
//  main.cpp
//  DiskArbitration demo
//
//  Created by Jozef on 12/05/2020.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//
// Source: https://developer.apple.com/library/archive/documentation/DriversKernelHardware/Conceptual/DiskArbitrationProgGuide/ArbitrationBasics/ArbitrationBasics.html

#include <iostream>
#include <sys/param.h>      // MAXPATHLEN
#include <DiskArbitration/DiskArbitration.h>
#include <DiskArbitration/DADisk.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

#include "../../Common/SignalHandler.hpp"

void got_disk(DADiskRef disk, void *context)
{
    std::cout << "New disk appeared >" << (DADiskGetBSDName(disk) ? DADiskGetBSDName(disk) : "") << "<." << std::endl;
}

void got_disk_removal(DADiskRef disk, void *context)
{
    std::cout << "Disk removed: >" << (DADiskGetBSDName(disk) ? DADiskGetBSDName(disk) : "") << "<." << std::endl;
}

void got_rename(DADiskRef disk, CFArrayRef keys, void *context)
{
    CFDictionaryRef dict = DADiskCopyDescription(disk);
    CFURLRef fspath = (CFURLRef)CFDictionaryGetValue(dict, kDADiskDescriptionVolumePathKey);
 
    char buf[MAXPATHLEN];
    if (CFURLGetFileSystemRepresentation(fspath, false, (UInt8 *)buf, sizeof(buf))) {
        std::cout << "Disk " << DADiskGetBSDName(disk) << " is now at " << buf << "\nChanged keys:" << std::endl;
        CFShow(keys);
    } else {
        /* Something is *really* wrong. */
    }
}

DADissenterRef allow_mount(DADiskRef disk, void *context)
{
        int allow = 0;
        if (allow) {
                /* Return NULL to allow */
                std::cerr << "allow_mount: allowing mount >" << (DADiskGetBSDName(disk) ? DADiskGetBSDName(disk) : "") << "<." << std::endl;
                return NULL;
        } else {
                /* Return a dissenter to deny */
                std::cerr << "allow_mount: refusing mount >" << (DADiskGetBSDName(disk) ? DADiskGetBSDName(disk) : "") << "<." << std::endl;
                return DADissenterCreate(
                        kCFAllocatorDefault,
                        kDAReturnExclusiveAccess,
                        CFSTR("USB Not Allowed To Mount!"));
        }
}


int main()
{
    InstallHandleSignalFromRunLoop();
    
    const char* demoName = "DiskArbitration";
    const std::string demoPath = "/tmp/" + std::string(demoName) + "-demo";
    
    std::cout << "(" << demoName << ") Hello, World!\n";
    std::cout << "Point of interest: " << "All the external USB devices!" << std::endl << std::endl;
    
    
    DASessionRef session = DASessionCreate(kCFAllocatorDefault);
    
    // Watch only USB devices
    CFMutableDictionaryRef matchingDict = CFDictionaryCreateMutable(
            kCFAllocatorDefault,
            0,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
    
    CFDictionaryAddValue(matchingDict,
            kDADiskDescriptionDeviceProtocolKey,
            CFSTR(kIOPropertyPhysicalInterconnectTypeUSB));
    
    void *context = NULL;
    DARegisterDiskAppearedCallback(session,
            kDADiskDescriptionMatchVolumeMountable,
            got_disk,
            context);
    
    /* Match all volumes */
    CFDictionaryRef matchingdictionary = NULL;
    
    /* No context needed here. */
    DARegisterDiskDisappearedCallback(session,
                                      matchingdictionary,
                                      got_disk_removal,
                                      context);
    
    CFMutableArrayRef keys = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
    CFArrayAppendValue(keys, kDADiskDescriptionVolumeNameKey);
     
    DARegisterDiskDescriptionChangedCallback(session,
                                             matchingdictionary, /* match all disks */
                                             keys, /* match the keys specified above */
                                             got_rename,
                                             context);
    
    DARegisterDiskMountApprovalCallback(session,
                                        NULL, /* Match all disks */
                                        allow_mount,
                                        NULL); /* No context */
    
    
    /* Schedule a disk arbitration session. */
    DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    
    
    
    /* Start the run loop.  (Don't do this if you already have
       a running Core Foundation or Cocoa run loop.) */
    CFRunLoopRun();
    
    
    DAUnregisterCallback(session, (void *)got_disk, context);
    DAUnregisterApprovalCallback(session, (void *)allow_mount, context);
    DAUnregisterCallback(session, (void *)got_disk_removal, context);
    DAUnregisterCallback(session, (void *)got_rename, context);
    
    /* Clean up a session. */
    DASessionUnscheduleFromRunLoop(session,
        CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    CFRelease(session);

    return EXIT_SUCCESS;
}
