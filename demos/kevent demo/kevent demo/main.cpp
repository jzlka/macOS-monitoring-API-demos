//
//  main.cpp
//  kevent demo
//
//  Created by Jozef on 12/05/2020.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//

// Sources:
// - https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/FSEvents_ProgGuide/KernelQueues/KernelQueues.html
// - https://developer.apple.com/library/archive/samplecode/FileNotification/Introduction/Intro.html

#include <atomic>           // std::atomic,
#include <cerrno>           // errno
#include <fcntl.h>          // O_EVTONLY
#include <iostream>         // std::cout, std::cerr,...
#include <sys/event.h>      // kqueue, kevent,...
#include <unistd.h>         // close

#define NUM_EVENT_SLOTS 1
#define NUM_EVENT_FDS 1
 
std::atomic<bool> g_shouldStop {false};
char *flagstring(int flags);

void signalHandler(int signum)
{
    // Not safe, but whatever
    std::cerr << "Interrupt signal (" << signum << ") received, exiting." << std::endl;
    g_shouldStop = true;
}


int main()
{
    // No runloop, no problem
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);

    const char* demoName = "kevent";
    const std::string demoPath = "/tmp/" + std::string(demoName) + "-demo";

    std::cout << "(" << demoName << ") Hello, World!\n";
    std::cout << "Point of interest: " << demoPath << std::endl << std::endl;
    
    /* Open a kernel queue. */
    int kq = kqueue();
    if (kq < 0) {
        std::cerr << "Could not open kernel queue.  Error was " << strerror(errno) << ".\n";
        return EXIT_FAILURE;
    }
 
    /*
       Open a file descriptor for the file/directory that you
       want to monitor.
     */
    int event_fd = open(demoPath.c_str(), O_EVTONLY);
    if (event_fd <=0) {
        std::cerr << "The file " << demoPath << " could not be opened for monitoring.  Error was "
                  << strerror(errno) << ".\n";
        return EXIT_FAILURE;
    }
 
    /*
       The address in user_data will be copied into a field in the
       event.  If you are monitoring multiple files, you could,
       for example, pass in different data structure for each file.
       For this example, the path string is used.
     */
    void *user_data = (void *)demoPath.c_str();
 
    /* Set the timeout to wake us every 5 and half second. */
    struct timespec timeout {
        .tv_sec = 5,            // 5 seconds
        .tv_nsec = 500000000    // 500 milliseconds
    };
 
    /* Set up a list of events to monitor. */
    unsigned int vnode_events;
    struct kevent events_to_monitor[NUM_EVENT_FDS];
    vnode_events = (NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE);
    EV_SET(&events_to_monitor[0], event_fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, vnode_events, 0, user_data);
 
    /* Handle events. */
    int num_files = 1;
    int continue_loop = 40; /* Monitor for twenty seconds. */
    struct kevent event_data[NUM_EVENT_SLOTS];
    while (--continue_loop && !g_shouldStop) {
        int event_count = kevent(kq, events_to_monitor, NUM_EVENT_SLOTS, event_data, num_files, &timeout);
        if ((event_count < 0) || (event_data[0].flags == EV_ERROR)) {
            /* An error occurred. */
            std::cerr << "An error occurred (event count " << event_count << ").  The error was "
                      << strerror(errno) << ".\n";
            break;
        }
        if (event_count) {
            std::cout << "Event " << event_data[0].ident << " occurred."
                      << " Filter " << event_data[0].filter
                      << ", flags " << event_data[0].flags
                      << ", filter flags " << flagstring(event_data[0].fflags)
                      << ", filter data " << event_data[0].data
                      << ", path " << (char *)event_data[0].udata << std::endl;
                
        } else {
            std::cout << "No event." << std::endl;
        }
 
        /* Reset the timeout.  In case of a signal interrruption, the
           values may change. */
        timeout.tv_sec = 5;             // 5 seconds
        timeout.tv_nsec = 500000000;    // 500 milliseconds
    }
    close(event_fd);
    return 0;
}


/* A simple routine to return a string for a set of flags. */
char *flagstring(int flags)
{
    static char ret[512];
    char *s_or = "";
 
    ret[0]='\0'; // clear the string.
    if (flags & NOTE_DELETE) {strcat(ret,s_or);strcat(ret,"NOTE_DELETE");s_or="|";}
    if (flags & NOTE_WRITE) {strcat(ret,s_or);strcat(ret,"NOTE_WRITE");s_or="|";}
    if (flags & NOTE_EXTEND) {strcat(ret,s_or);strcat(ret,"NOTE_EXTEND");s_or="|";}
    if (flags & NOTE_ATTRIB) {strcat(ret,s_or);strcat(ret,"NOTE_ATTRIB");s_or="|";}
    if (flags & NOTE_LINK) {strcat(ret,s_or);strcat(ret,"NOTE_LINK");s_or="|";}
    if (flags & NOTE_RENAME) {strcat(ret,s_or);strcat(ret,"NOTE_RENAME");s_or="|";}
    if (flags & NOTE_REVOKE) {strcat(ret,s_or);strcat(ret,"NOTE_REVOKE");s_or="|";}
 
    return ret;
}

