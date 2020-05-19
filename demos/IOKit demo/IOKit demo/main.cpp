//
//  main.cpp
//  IOKit demo
//
//  Created by Jozef on 09/08/2019.
//  Copyright © 2019 Jozef Zuzelka. All rights reserved.
//
// Source: OS X and iOS Kernel Programming

#include <CoreFoundation/CoreFoundation.h>
#include <iostream>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <utility>

#include "../../../Common/SignalHandler.hpp"

// Structure to describe a driver instance.
struct MyDriverData {
    io_service_t    service;
    io_object_t     notification;
} ;


class IOKitMonitorTrampoline
{
public:
    static void DeviceAdded( void* _Nonnull  context, io_iterator_t iterator);
    static void DeviceNotification ( void* _Nonnull  context, io_service_t service, natural_t messageType, void* _Nonnull messageArgument);
};

class IOKitMonitor
{
    _Nullable CFDictionaryRef matching_dict = NULL;
    /// Iterator which is set when a IOService match is found
    io_iterator_t iter = 0;
    /// Notification port used for both device arrival and driver state changes
    _Nullable IONotificationPortRef notification_port = NULL;
    /// A run loop source used for receiving device notifications
    _Nullable CFRunLoopSourceRef event_source = NULL;
    
public:
    void DeviceAdded(void* _Nonnull context, io_iterator_t iterator);
    void DeviceNotification (void* _Nonnull context, io_service_t service, natural_t messageType, void* _Nonnull messageArgument);
    void Init();
    void Uninit();
    
};


void IOKitMonitorTrampoline::DeviceAdded(void* context, io_iterator_t iterator)
{
    if (context == nullptr)
        return;
    static_cast<IOKitMonitor*>(context)->DeviceAdded(context, iterator);
}

void IOKitMonitorTrampoline::DeviceNotification(void* context, io_service_t service, natural_t messageType, void* messageArgument)
{
    if (context == nullptr)
        return;
    
    // Context contains a pointer to pair of pointers to IOKit monitor class and driver data, respectively.
    auto *data = static_cast<std::pair<IOKitMonitor*, MyDriverData*> *>(context);
    IOKitMonitor * const iokit_monitor = data->first;
    
    if (iokit_monitor == nullptr)
        return;
    
    iokit_monitor->DeviceNotification(data, service, messageType, messageArgument);
}

    
void IOKitMonitor::Init()
{
    // Create a matching dictionary that will find any USB device
    matching_dict = IOServiceMatching("IOUSBDevice");
    if (!matching_dict)
        return;
    
    notification_port = IONotificationPortCreate(kIOMasterPortDefault);
    event_source = IONotificationPortGetRunLoopSource(notification_port);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), event_source, kCFRunLoopDefaultMode);
    kern_return_t kr = IOServiceAddMatchingNotification(notification_port, kIOFirstMatchNotification,
                                          matching_dict, IOKitMonitorTrampoline::DeviceAdded, this, &iter);
    
    // Remove our interest in the source. Now the run loop is responsible for retaining it until it's done
    CFRelease(event_source);
    
    // Arm the notification
    IOKitMonitorTrampoline::DeviceAdded(this, iter);
}
    
void IOKitMonitor::Uninit()
{
    // Release the iterator
    if (iter) {
        IOObjectRelease(iter);
        iter = 0;
    }
    
    if (notification_port) {
        // http://web.archive.org/web/20140115103252/https://lists.apple.com/archives/darwin-drivers/2004/Sep/msg00058.html
        // We should call IONotificationPortDestroy() since it releases the runloop source. However we
        // got this source by calling IONotificationPortGetRunLoopSource() and following
        // "The Create Rule" we are not supposed to release it \o/.
        // So DO NOT call IONotificationPortDestroy(notification_port);!
        notification_port = NULL;
    }
    
    if (event_source) {
        //CFRunLoopRemoveSource     Removes the source from the specific run loop you specify.
        // -- this might cause a crash if the runloop does not exist during this call (i.e. this
        //    thread received a signal? - and uninit is called when runloop is closed)
        
        //CFRunLoopSourceInvalidate Renders the source invalid, and will remove it from all the run
        //                          loops where was added.
        // -- this removes the source from all the run loops it was added to, ignoring run loops that
        //    now do not exist anymore.
        CFRunLoopSourceInvalidate(event_source);
        
        // The caller should not release this CFRunLoopSource. Just call IONotificationPortDestroy
        // to dispose of the IONotificationPortRef and the CFRunLoopSource when done.
        // (!!! documentation is wrong at this point (see releasing of notification_port comment below)
        // So DO NOT call CFRelease(event_source);!
        event_source = NULL;
    }
}
 
void IOKitMonitor::DeviceAdded(void* refCon, io_iterator_t iterator)
{
    io_service_t service = 0;
    // Iterate over all matching objects.
    while ((service = IOIteratorNext(iterator)) != 0)
    {
        CFStringRef className;
        // List all IOUSBDevice objects, ignoring objects that subclass IOUSBDevice.
        className = IOObjectCopyClass(service);
        
        // Because of calling this function during inicialization to arm notifications, devices which are not USBs
        // might appear here as well.
        if (CFEqual(className, CFSTR("IOUSBDevice")) == true)
        {
            io_name_t name;
            IORegistryEntryGetName(service, name);
            std::cout << "Found device with name: " << name << std::endl;
            
            std::pair<IOKitMonitor*, std::unique_ptr<MyDriverData>> *data = new std::pair<IOKitMonitor*, std::unique_ptr<MyDriverData>>(this, std::make_unique<MyDriverData>());
            // Save the io_service_t for this driver instance.
            data->second->service = service;
            // Install a callback to receive notification of driver state changes.
            kern_return_t kr = IOServiceAddInterestNotification(notification_port,
                                                  service, // driver object
                                                  kIOGeneralInterest,
                                                  IOKitMonitorTrampoline::DeviceNotification, // callback
                                                  data, // refCon passed to callback
                                                  &(data->second->notification));
            if (kr != KERN_SUCCESS) {
                delete data;
                continue;
            }
        }
        CFRelease(className);
    }
}

void IOKitMonitor::DeviceNotification(void* refCon, io_service_t service, natural_t messageType,
                         void* messageArgument)
{
    if (refCon == nullptr)
        return;
    
    auto *myDriverData = static_cast<std::pair<IOKitMonitor*, std::unique_ptr<MyDriverData>> *>(refCon);

    // Only handle driver termination notifications.
    if (messageType == kIOMessageServiceIsTerminated)
    {
        // Print the name of the removed device.
        io_name_t  name;
        IORegistryEntryGetName(service, name);
        std::cout << "Device removed: " << name << std::endl;;
        
        // Remove the driver state change notification.
        kern_return_t kr = IOObjectRelease(myDriverData->second->notification);
        // Release our reference to the driver object.
        IOObjectRelease(myDriverData->second->service);
        // Release our structure that holds the driver connection.
        delete myDriverData;
    }
}

int main(int argc, const char * argv[])
{
    InstallHandleSignalFromRunLoop();
    
    const char* demoName = "IOKit";
    const std::string demoPath = "/tmp/" + std::string(demoName) + "-demo";
    
    std::cout << "(" << demoName << ") Hello, World!\n";
    std::cout << "Point of interest: " << "All the external USB devices!\n" << std::endl;
    
    IOKitMonitor iom;
    iom.Init();
    
    CFRunLoopRun();

    iom.Uninit();
    return 0;
}
