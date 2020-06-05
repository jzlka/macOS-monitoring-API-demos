//
//  SignalHandler.cpp
//  
//
//  Created by Jozef on 11/05/2020.
//

#include <CoreFoundation/CoreFoundation.h>
#include <sys/event.h>
#include <iostream>
#include <vector>

#include "logger.hpp"
#include "SignalHandler.hpp"

// Source: https://github.com/HelmutJ/CocoaSampleCode/blob/master/PreLoginAgents/PreLoginAgentCarbon/main.c & Mark Dalrymple, Advanced Mac OS X Programming: The Big Nerd Ranch Guide
void HandleSignalFromRunLoop(CFFileDescriptorRef f, CFOptionFlags callBackTypes, void *info);
    // forward declaration

void InstallHandleSignalFromRunLoop()
    // This routine installs HandleSIGTERMFromRunLoop as a SIGTERM handler.
    // The wrinkle is, HandleSIGTERMFromRunLoop is called from the runloop rather
    // than as a signal handler.  This means that HandleSIGTERMFromRunLoop is not
    // limited to calling just the miniscule set of system calls that are safe
    // from a signal handler.
    //
    // This routine leaks lots of stuff.  You're only expected to call it once,
    // from the main thread of your program.
{
    // Ignore signals.  Even though we've ignored the signal, the kqueue will
    // still see it.
    const std::vector<int> signalsToWatch = {SIGINT, SIGABRT, SIGTERM};
    // For SIGSEGV see
    // https://stackoverflow.com/questions/16204271/about-catching-the-sigsegv-in-multithreaded-environment
    // https://stackoverflow.com/questions/6533373/is-sigsegv-delivered-to-each-thread/6533431#6533431
    // https://stackoverflow.com/questions/20304720/catching-signals-such-as-sigsegv-and-sigfpe-in-multithreaded-program
    sig_t sigErr;
    for (const auto &signum : signalsToWatch)
    {
        sigErr = signal(signum, SIG_IGN);
        assert(sigErr != SIG_ERR);
    }
    sigErr = signal(SIGPIPE, SIG_IGN);
    assert(sigErr != SIG_ERR);
    
    // Create a kqueue and configure it to listen for the selected signals.
    int kq = kqueue();
    assert(kq >= 0);
    
    // Use the new-in-10.5 EV_RECEIPT flag to ensure that we get what we expect.
    for (const auto &signum : signalsToWatch)
    {
        struct kevent sigevent;
        EV_SET(&sigevent, signum, EVFILT_SIGNAL, EV_ADD | EV_RECEIPT, 0, 0, NULL);
        int changeCount = kevent(kq, &sigevent, 1, &sigevent, 1, NULL);
        assert(changeCount == 1);           // check that we get an event back
        assert(sigevent.flags & EV_ERROR);   // and that it contains error information
        assert(sigevent.data == 0);          // with no error
    }
    
    // Wrap the kqueue in a CFFileDescriptor (new in Mac OS X 10.5!).  Then
    // create a run-loop source from the CFFileDescriptor and add that to the
    // runloop.
    static const CFFileDescriptorContext kContext = { 0, NULL, NULL, NULL, NULL };
    CFFileDescriptorRef kqRef = CFFileDescriptorCreate(NULL, kq, true, HandleSignalFromRunLoop, &kContext);
    assert(kqRef != NULL);
    
    CFRunLoopSourceRef kqSource = CFFileDescriptorCreateRunLoopSource(NULL, kqRef, 0);
    assert(kqSource != NULL);
    
    CFRunLoopAddSource(CFRunLoopGetCurrent(), kqSource, kCFRunLoopDefaultMode);
    
    CFFileDescriptorEnableCallBacks(kqRef, kCFFileDescriptorReadCallBack);

    // Clean up.  We can release kqSource and kqRef because they're all being
    // kept live by the fact that the kqSource is added to the runloop.  We
    // must not close kq because file descriptors are not reference counted
    // and kqRef now 'owns' this descriptor.
    CFRelease(kqSource);
    CFRelease(kqRef);
}

void HandleSignalFromRunLoop(CFFileDescriptorRef f, CFOptionFlags callBackTypes, void *info)
    // Called from the runloop when the process receives a SIGTERM.  We log
    // this occurence (which is safe to do because we're not in an actual signal
    // handler, courtesy of the 'magic' in InstallHandleSIGTERMFromRunLoop)
    // and then tell the app to quit.
{
    #pragma unused(f)
    #pragma unused(callBackTypes)
    #pragma unused(info)

    Logger::getInstance().log(LogLevel::INFO, "(☞ﾟヮﾟ)☞ Interrupt signal () received, exiting ฅ^•ﻌ•^ฅ.");
    CFRunLoopStop(CFRunLoopGetCurrent());
}
