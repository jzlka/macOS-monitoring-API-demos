//
//  SignalHandler.cpp
//  
//
//  Created by Jozef on 11/05/2020.
//

#include <sys/event.h>

#include "SignalHandler.hpp"

// Source: https://github.com/HelmutJ/CocoaSampleCode/blob/master/PreLoginAgents/PreLoginAgentCarbon/main.c
void HandleSIGTERMFromRunLoop(CFFileDescriptorRef f, CFOptionFlags callBackTypes, void *info);
    // forward declaration

void InstallHandleSIGTERMFromRunLoop()
    // This routine installs HandleSIGTERMFromRunLoop as a SIGTERM handler.
    // The wrinkle is, HandleSIGTERMFromRunLoop is called from the runloop rather
    // than as a signal handler.  This means that HandleSIGTERMFromRunLoop is not
    // limited to calling just the miniscule set of system calls that are safe
    // from a signal handler.
    //
    // This routine leaks lots of stuff.  You're only expected to call it once,
    // from the main thread of your program.
{
    static const CFFileDescriptorContext    kContext = { 0, NULL, NULL, NULL, NULL };
    sig_t                   sigErr;
    int                     kq;
    CFFileDescriptorRef     kqRef;
    CFRunLoopSourceRef      kqSource;
    struct kevent           changes;
    int                     changeCount;
    
    // Ignore SIGTERM.  Even though we've ignored the signal, the kqueue will
    // still see it.
    
    sigErr = signal(SIGTERM, SIG_IGN);
    assert(sigErr != SIG_ERR);
    
    // Create a kqueue and configure it to listen for the SIGTERM signal.
    
    kq = kqueue();
    assert(kq >= 0);
    
    // Use the new-in-10.5 EV_RECEIPT flag to ensure that we get what we expect.
    
    EV_SET(&changes, SIGTERM, EVFILT_SIGNAL, EV_ADD | EV_RECEIPT, 0, 0, NULL);
    changeCount = kevent(kq, &changes, 1, &changes, 1, NULL);
    assert(changeCount == 1);           // check that we get an event back
    assert(changes.flags & EV_ERROR);   // and that it contains error information
    assert(changes.data == 0);          // with no error
    
    // Wrap the kqueue in a CFFileDescriptor (new in Mac OS X 10.5!).  Then
    // create a run-loop source from the CFFileDescriptor and add that to the
    // runloop.
    
    kqRef = CFFileDescriptorCreate(NULL, kq, true, HandleSIGTERMFromRunLoop, &kContext);
    assert(kqRef != NULL);
    
    kqSource = CFFileDescriptorCreateRunLoopSource(NULL, kqRef, 0);
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

void HandleSIGTERMFromRunLoop(CFFileDescriptorRef f, CFOptionFlags callBackTypes, void *info)
    // Called from the runloop when the process receives a SIGTERM.  We log
    // this occurence (which is safe to do because we're not in an actual signal
    // handler, courtesy of the 'magic' in InstallHandleSIGTERMFromRunLoop)
    // and then tell the app to quit.
{
    #pragma unused(f)
    #pragma unused(callBackTypes)
    #pragma unused(info)

    CFRunLoopStop(CFRunLoopGetCurrent());
}
