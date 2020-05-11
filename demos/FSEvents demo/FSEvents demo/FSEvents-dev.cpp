//
//  FSEvents-dev.cpp
//  FSEvents demo
//
//  Created by Jozef on 11/05/2020.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//

// Source: Jonathan Levin, TWTR: @Morpheus______ http://NewOSXBook.com/
// For details, q.v. MOXiI 1st Ed, chapter 3 (pp 74-78), or MOXiI II, Volume I, Chapter 5


#include <iostream>
#include <map>

#include <fcntl.h>        // O_RDONLY
#include <sys/ioctl.h>    // for _IOW, a macro required by FSEVENTS_CLONE
#include <sys/sysctl.h>   // for sysctl, KERN_PROC, etc.
#include <unistd.h>       // geteuid, read, close

#include "fsevents.h"     // copied from xnu//bsd/sys/fsevents.h

void signalHandler(int signum)
{
    // Not safe, but whatever
    std::cerr << "Interrupt signal (" << signum << ") received, exiting." << std::endl;
    exit(signum);
}

#define BUFSIZE 1024 *1024

#define COLOR_OP YELLOW
#define COLOR_PROC BLUE
#define COLOR_PATH CYAN

static inline const std::map<uint32_t, std::string> g_eventToStringMap = {
    {FSE_CREATE_FILE,         "Created       "},
    {FSE_DELETE,              "Deleted       "},
    {FSE_STAT_CHANGED,        "Changed stat  "},
    {FSE_RENAME,              "Renamed       "},
    {FSE_CONTENT_MODIFIED,    "Modified      "},
    {FSE_CREATE_DIR,          "Created dir   "},
    {FSE_CHOWN,               "Chowned       "},
    {FSE_EXCHANGE,            "Exchanged     "},
    {FSE_FINDER_INFO_CHANGED, "Finder Info   "},
    {FSE_XATTR_MODIFIED,      "Changed xattr "},
    {FSE_XATTR_REMOVED,       "Removed xattr "},
    {FSE_DOCID_CREATED,       "DocID created "},
    {FSE_DOCID_CHANGED,       "DocID changed "},
};

struct kfs_event_arg {
    /* argument type */
    u_int16_t  type;

    /* size of argument data that follows this field */
    u_int16_t  len;

    union {
        struct vnode *vp;
        char    *str;
        void    *ptr;
        int32_t  int32;
        dev_t    dev;
        ino_t    ino;
        int32_t  mode;
        uid_t    uid;
        gid_t    gid;
        uint64_t timestamp;
    } data;
};

typedef struct kfs_event_a {
    uint16_t type;
    uint16_t refcount;
    pid_t    pid;
} kfs_event_a;

int lastPID = 0;
static char *getProcName(long pid)
{
    static char procName[4096];
    size_t len = 1000;
    int rc;
    int mib[4];

    // minor optimization
    if (pid != lastPID)
    {
        memset(procName, '\0', 4096);

        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PID;
        mib[3] = pid;

        if ((rc = sysctl(mib, 4, procName, &len, NULL,0)) < 0)
        {
            perror("trace facility failure, KERN_PROC_PID\n");
            exit(1);
        }

        //printf ("GOT PID: %d and rc: %d -  %s\n", mib[3], rc, ((struct kinfo_proc *)procName)->kp_proc.p_comm);
        lastPID = pid;
    }
    return (((struct kinfo_proc *)procName)->kp_proc.p_comm);
}


int main()
{
    // No runloop, no problem
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGPIPE, SIG_IGN);
    //signal(SIGSEGV, signalHandler); // thread specific, see
    // https://stackoverflow.com/questions/16204271/about-catching-the-sigsegv-in-multithreaded-environment
    // https://stackoverflow.com/questions/6533373/is-sigsegv-delivered-to-each-thread/6533431#6533431
    // https://stackoverflow.com/questions/20304720/catching-signals-such-as-sigsegv-and-sigfpe-in-multithreaded-program
    
    int fsed, cloned_fsed;
    // Open the device
    fsed = open ("/dev/fsevents", O_RDONLY);
    
    if (geteuid())
        std::cerr << "Opening /dev/fsevents requires root permissions\n";

    if (fsed < 0)
    {
        std::cerr << "Could not open the device\n";
        exit(1);
    }

    // Prepare event mask list. In our simple example, we want everything
    // (i.e. all events, so we say "FSE_REPORT" all). Otherwise, we
    // would have to specifically toggle FSE_IGNORE for each:
    //
    // e.g.
    //       events[FSE_XATTR_MODIFIED] = FSE_IGNORE;
    //       events[FSE_XATTR_REMOVED]  = FSE_IGNORE;
    // etc..
    int8_t events[FSE_MAX_EVENTS];
    for (int i=0; i < FSE_MAX_EVENTS; i++)
        events[i] = FSE_REPORT;
    
    fsevent_clone_args  clone_args;
    
    // Get ready to clone the descriptor:
    memset(&clone_args, '\0', sizeof(clone_args));
    clone_args.fd = &cloned_fsed; // This is the descriptor we get back
    clone_args.event_queue_depth = 100; // Makes all the difference
    clone_args.event_list = events;
    clone_args.num_events = FSE_MAX_EVENTS;
    
    // Do it.
    int rc = ioctl (fsed, FSEVENTS_CLONE, &clone_args);
    
    if (rc < 0)
    {
        std::cerr << "ioctl error.\n";
        exit(2);
    }
    
    // We no longer need original..
    close (fsed);
    
    char buf[BUFSIZE];
    // And now we simply read, ad infinitum (aut nauseam)
    while ((rc = read (cloned_fsed, buf, BUFSIZE)) > 0)
    {
        // rc returns the count of bytes for one or more events:
        int offInBuf = 0;

        while (offInBuf < rc)
        {
            std::cout << "----" << offInBuf << "/" << rc << " bytes.\n";
            
            struct kfs_event_a *fse = (struct kfs_event_a *)(buf + offInBuf);
            struct kfs_event_arg *fse_arg;
            
            if (fse->type == FSE_EVENTS_DROPPED)
            {
                std::cout << "Some events dropped\n";
                break;
            }
            
            if (!fse->pid)
                std::cout << std::hex << "Type: 0x" << fse->type << "RefCount: 0x" << fse->refcount << std::dec << std::endl;
            
            char *procName = getProcName(fse->pid);
            offInBuf+= sizeof(struct kfs_event_a);
            fse_arg = (struct kfs_event_arg *) &buf[offInBuf];
            
            std::cout << "PID: " << fse->pid << " Process name: " << procName << " Event type: " << g_eventToStringMap.at(fse->type);

            // Crashing...
            //if (fse_arg->type == FSE_ARG_STRING)
            //    std::cout << "Path: " << fse_arg->data.str << std::endl;
        }
        
        if (rc > offInBuf)
            std::cout << "*** Warning: Some events may be lost." << std::endl;
    }
}
