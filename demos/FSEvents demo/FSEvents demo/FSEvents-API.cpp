//
//  FSEvents-API.cpp
//  FSEvents demo
//
//  Created by Jozef on 11/05/2020.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//

#include <iostream>
#include <map>
#include "../../Common/SignalHandler.hpp"

#include <CoreServices/CoreServices.h>

static inline const std::map<UInt32, const std::string> g_streamEventFlags = {
    {kFSEventStreamEventFlagNone,               "kFSEventStreamEventFlagNone"},
    {kFSEventStreamEventFlagMustScanSubDirs,    "kFSEventStreamEventFlagMustScanSubDirs"},
    {kFSEventStreamEventFlagUserDropped,        "kFSEventStreamEventFlagUserDropped"},
    {kFSEventStreamEventFlagKernelDropped,      "kFSEventStreamEventFlagKernelDropped"},
    {kFSEventStreamEventFlagEventIdsWrapped,    "kFSEventStreamEventFlagEventIdsWrapped"},
    {kFSEventStreamEventFlagHistoryDone,        "kFSEventStreamEventFlagHistoryDone"},
    {kFSEventStreamEventFlagRootChanged,        "kFSEventStreamEventFlagRootChanged"},
    {kFSEventStreamEventFlagMount,              "kFSEventStreamEventFlagMount"},
    {kFSEventStreamEventFlagUnmount,            "kFSEventStreamEventFlagUnmount"},
    {kFSEventStreamEventFlagItemCreated,        "kFSEventStreamEventFlagItemCreated"},
    {kFSEventStreamEventFlagItemRemoved,        "kFSEventStreamEventFlagItemRemoved"},
    {kFSEventStreamEventFlagItemInodeMetaMod,   "kFSEventStreamEventFlagItemInodeMetaMod"},
    {kFSEventStreamEventFlagItemRenamed,        "kFSEventStreamEventFlagItemRenamed"},
    {kFSEventStreamEventFlagItemModified,       "kFSEventStreamEventFlagItemModified"},
    {kFSEventStreamEventFlagItemFinderInfoMod,  "kFSEventStreamEventFlagItemFinderInfoMod"},
    {kFSEventStreamEventFlagItemChangeOwner,    "kFSEventStreamEventFlagItemChangeOwner"},
    {kFSEventStreamEventFlagItemXattrMod,       "kFSEventStreamEventFlagItemXattrMod"},
    {kFSEventStreamEventFlagItemIsFile,         "kFSEventStreamEventFlagItemIsFile"},
    {kFSEventStreamEventFlagItemIsDir,          "kFSEventStreamEventFlagItemIsDir"},
    {kFSEventStreamEventFlagItemIsSymlink,      "kFSEventStreamEventFlagItemIsSymlink"},
    {kFSEventStreamEventFlagOwnEvent,           "kFSEventStreamEventFlagOwnEvent"},
    {kFSEventStreamEventFlagItemIsHardlink,     "kFSEventStreamEventFlagItemIsHardlink"},
    {kFSEventStreamEventFlagItemIsLastHardlink, "kFSEventStreamEventFlagItemIsLastHardlink"},
    {kFSEventStreamEventFlagItemCloned,         "kFSEventStreamEventFlagItemCloned"},
};

void eventCallback(
    ConstFSEventStreamRef streamRef, // The stream for which event(s) occured.
    void *clientCallBackInfo,        // Context.
    size_t numEvents,                // The number of events being reported in this callback.
    void *eventPaths,                // An array of paths to the directories in which event(s) occured.
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[])
{
    const char * const *paths = (char**)eventPaths;
 
    for (int i=0; i<numEvents; i++)
    {
        std::string eventDesc = "{";
        for (const auto &[key, desc] : g_streamEventFlags)
            eventDesc += (eventFlags[i] & key) ? (desc + ",") : "";
        eventDesc += "}";
        
        std::cout << "Change " << eventIds[i] << " in " << paths[i] << ", flags " << eventFlags[i] << ":" << eventDesc << std::endl;
    }
}

int main() {
    InstallHandleSignalFromRunLoop();
    
    const char* demoName = "FSEvents-API";
    const std::string demoPath = "/tmp/" + std::string(demoName) + "-demo";
    
    std::cout << "(" << demoName << ") Hello, World!\n";
    std::cout << "Point of interest: " << demoPath << std::endl << std::endl;
    
    // Init source paths to watch
    CFStringRef path  = CFStringCreateWithCString(kCFAllocatorDefault, demoPath.c_str(), kCFStringEncodingUTF8);
    CFArrayRef pathsToWatch = CFArrayCreate(kCFAllocatorDefault, (const void **)&path, 1, NULL);
    // Set context passed to the stream callback
    FSEventStreamContext *ctx = nullptr;
    // Set latency in seconds
    CFAbsoluteTime latency = 3.0;
    
    FSEventStreamRef stream = FSEventStreamCreate(kCFAllocatorDefault,
                                                  eventCallback,
                                                  ctx,
                                                  pathsToWatch,
                                                  kFSEventStreamEventIdSinceNow, // Or a previous event ID
                                                  latency,
                                                  kFSEventStreamCreateFlagIgnoreSelf | kFSEventStreamCreateFlagFileEvents);
    
    FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    
    // Tell the daemon to send notifications to our client
    FSEventStreamStart(stream);
    
    CFRunLoopRun();
    
    // Stop receaving the notifications
    FSEventStreamStop(stream);
    // Unschedule the stream from all run loops
    FSEventStreamInvalidate(stream);
    // Release resources
    FSEventStreamRelease(stream);
    CFRelease(path);
    CFRelease(pathsToWatch);
    
    return 0;
}
