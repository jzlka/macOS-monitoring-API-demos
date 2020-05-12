//
//  FSEvents-dev.cpp
//  FSEvents demo
//
//  Created by Jozef on 11/05/2020.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//

// Source: Jonathan Levin, TWTR: @Morpheus______ http://NewOSXBook.com/ & Amit Singh, Mac OS X Internals: A Systems Approach
// For details, q.v. MOXiI 1st Ed, chapter 3 (pp 74-78), or MOXiI II, Volume I, Chapter 5


#include <iostream>
#include <map>

#include <fcntl.h>        // O_RDONLY
#include <unistd.h>       // geteuid, read, close
#include <sys/ioctl.h>    // for _IOW, a macro required by FSEVENTS_CLONE
//#include <sys/types.h>
#include <sys/sysctl.h>   // for sysctl, KERN_PROC, etc.
#include "fsevents.h"     // copied from xnu//bsd/sys/fsevents.h
#include <pwd.h>
#include <grp.h>

void signalHandler(int signum)
{
    // Not safe, but whatever
    std::cerr << "Interrupt signal (" << signum << ") received, exiting." << std::endl;
    exit(signum);
}

#define BUFSIZE 1024 *1024

static inline const std::map<uint32_t, std::string> g_kfseNames = {
    {FSE_CREATE_FILE,         "FSE_CREATE_FILE"},
    {FSE_DELETE,              "FSE_DELETE"},
    {FSE_STAT_CHANGED,        "FSE_STAT_CHANGED"},
    {FSE_RENAME,              "FSE_RENAME"},
    {FSE_CONTENT_MODIFIED,    "FSE_CONTENT_MODIFIED"},
    {FSE_CREATE_DIR,          "FSE_CREATE_DIR"},
    {FSE_CHOWN,               "FSE_CHOWN"},
    {FSE_EXCHANGE,            "FSE_EXCHANGE"},
    {FSE_FINDER_INFO_CHANGED, "FSE_FINDER_INFO_CHANGED"},
    {FSE_XATTR_MODIFIED,      "FSE_XATTR_MODIFIED"},
    {FSE_XATTR_REMOVED,       "FSE_XATTR_REMOVED"},
    {FSE_DOCID_CREATED,       "FSE_DOCID_CREATED"},
    {FSE_DOCID_CHANGED,       "FSE_DOCID_CHANGED"},
};

static inline const std::map<uint32_t, std::string> g_kfseArgNames = {
    {0,                 "Unknown"},
    {FSE_ARG_VNODE,     "FSE_ARG_VNODE"},
    {FSE_ARG_STRING,    "FSE_ARG_STRING"},
    {FSE_ARG_PATH,      "FSE_ARG_PATH"},
    {FSE_ARG_INT32,     "FSE_ARG_INT32"},
    {FSE_ARG_INT64,     "FSE_ARG_INT64"},
    {FSE_ARG_RAW,       "FSE_ARG_RAW"},
    {FSE_ARG_INO,       "FSE_ARG_INO"},
    {FSE_ARG_UID,       "FSE_ARG_UID"},
    {FSE_ARG_DEV,       "FSE_ARG_DEV"},
    {FSE_ARG_MODE,      "FSE_ARG_MODE"},
    {FSE_ARG_GID,       "FSE_ARG_GID"},
    {FSE_ARG_FINFO,     "FSE_ARG_FINFO"},
};

struct kfs_event_arg_t {
    /* argument type */
    u_int16_t  type;

    /* size of argument data that follows this field */
    u_int16_t  len;

    union {
        struct vnode *vp;
        char    *str;
        void    *ptr;
        int32_t  int32;
        int64_t  int64;
        dev_t    dev;
        ino_t    ino;
        int32_t  mode;
        uid_t    uid;
        gid_t    gid;
        uint64_t timestamp;
    } data;
};

#define KFS_NUM_ARGS FSE_MAX_ARGS
// an event
struct kfs_event {
    int32_t type; // Event type
    pid_t pid; // pid of the process that performed the operation
    kfs_event_arg_t args[KFS_NUM_ARGS]; // event arguments
};

// for pretty-printing of vnode types
enum vtype {
    VNON, VREG, VDIR, VBLK, VCHR, VLNK, VSOCK, VFIFO, VBAD, VSTR, VCPLX
};

enum vtype iftovt_tab[] = {
    VNON, VFIFO, VCHR, VNON, VDIR, VNON, VBLK, VNON,
    VREG, VNON, VLNK, VNON, VSOCK, VNON, VNON, VBAD,
};

static const char *vtypeNames[] = {
    "VNON", "VREG", "VDIR", "VBLK", "VCHR", "VLNK",
    "VSOCK", "VFIFO", "VBAD", "VSTR", "VCPLX",
};
#define VTYPE_MAX (sizeof(vtypeNames)/sizeof(char *))

int lastPID = 0;
static char *getProcName(long pid)
{
    static char procName[4096];
    size_t len = 1000;
    int rc;
    int mib[4];

    // minor optimization
    if (pid != lastPID) {
        memset(procName, '\0', 4096);

        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PID;
        mib[3] = pid;

        if ((rc = sysctl(mib, 4, procName, &len, NULL,0)) < 0) {
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
    
    const char* demoName = "FSEvents-dev";
    const std::string demoPath = "/tmp/" + std::string(demoName) + "-demo";
    
    std::cout << "(" << demoName << ") Hello, World!\n";
    std::cout << "Point of interest: " << "All the events!" << std::endl << std::endl;
    
    int fsed, cloned_fsed;
    // Open the device
    fsed = open ("/dev/fsevents", O_RDONLY);
    
    if (geteuid())
        std::cerr << "Opening /dev/fsevents requires root permissions\n";

    if (fsed < 0) {
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
    if (rc < 0) {
        std::cerr << "ioctl error.\n";
        close(fsed);
        exit(2);
    }
    
    // We no longer need original..
    close(fsed);
    
    u_int32_t is_fse_arg_vnode = 0;
    char buf[BUFSIZE];
    // And now we simply read, ad infinitum (aut nauseam)
    while ((rc = read (cloned_fsed, buf, BUFSIZE)) > 0) { // event-processing loop
        // rc returns the count of bytes for one or more events:
        int off = 0;

        while (off < rc) {
            std::cout << "----" << off << "/" << rc << " bytes.\n";
            
            kfs_event *kfse = (kfs_event *)((char*)buf + off);
            off += sizeof(kfse->type) + sizeof(kfse->pid);
            
            
            if (kfse->type == FSE_EVENTS_DROPPED) {
                std::cout << "#Event\n" << "\ttype = " << "EVENTS DROPPED" << "\n\tpid = " << kfse->pid << std::endl;
                off += sizeof(u_int16_t); // FSE_ARG_DONE: sizeof(type)
                continue;
            }
            
            if (kfse->type < FSE_MAX_EVENTS && kfse->type >= -1) {
                std::cout << "#Event\n" << "\ttype = " << g_kfseNames.at(kfse->type) << "\n\tpid = " << kfse->pid << " (" << (getProcName(kfse->pid) ? getProcName(kfse->pid) : "?") << ")" << std::endl;
            }
            
            std::cout << "\t#Details\n\tType\t\tLength\tData" << std::endl;
            kfs_event_arg_t *kea = kfse->args;
            
            int i = 0;
            while ((off < rc) && (i <= FSE_MAX_ARGS)) { // process arguments
                i++;
                
                if (kea->type == FSE_ARG_DONE) { // no more arguments
                    std::cout << "\t" << "FSE_ARG_DONE\t" << kea->type << std::endl;
                    off += sizeof(kea->type);
                    break;
                }
                
                int eoff = sizeof(kea->type) + sizeof(kea->len) + kea->len;
                off += eoff;
                
                int32_t arg_id = (kea->type > FSE_MAX_ARGS) ? 0 : kea->type;
                std::cout << "\t" << g_kfseArgNames.at(arg_id) << "\t\t" << kea->len << "\t";
                
                switch (kea->type) { // handle based on argument type
                    case FSE_ARG_VNODE:     // a vnode (string) pointer
                        is_fse_arg_vnode = 1;
                        std::cout << "path = " << (char*)&(kea->data.vp) << std::endl;
                        break;
                    case FSE_ARG_STRING:    // a string pointer
                        std::cout << "string = " << (char*)&(kea->data.str) << std::endl;
                        break;
                    case FSE_ARG_INT32:
                        std::cout << "int32 = " << kea->data.int32 << std::endl;
                        break;
                    case FSE_ARG_INT64:
                        std::cout << "int64 = " << kea->data.int64 << std::endl;
                        break;
                    case FSE_ARG_RAW:       // a void pointer
                        std::cout << "ptr = " << std::hex << "0x" << (long)(kea->data.ptr) << std::dec << std::endl;
                        break;
                    case FSE_ARG_INO:       // an inode number
                        std::cout << "ino = " << kea->data.ino << std::endl;
                        break;
                    case FSE_ARG_UID:       // a user ID
                    {
                        struct passwd *p = getpwuid(kea->data.uid);
                        std::cout << "uid = " << kea->data.uid << "(" << ((p) ? p->pw_name : "?") << ")" << std::endl;
                        break;
                    }
                    case FSE_ARG_DEV:       // a file system ID or a device number
                        if (is_fse_arg_vnode) {
                            std::cout << "fsid = " << std::hex << kea->data.dev << std::dec << std::endl;
                            is_fse_arg_vnode = 0;
                        } else {
                            std::cout << "dev = " << std::hex << kea->data.dev << std::dec << "(major " << major(kea->data.dev) << " minor " << minor(kea->data.dev) << ")" << std::endl;
                        }
                        break;
                    case FSE_ARG_MODE:      // a combination of file mode and file type
                    {
                        mode_t va_mode = (kea->data.mode & 0x0000ffff);
                        u_int32_t va_type = (kea->data.mode & 0xfffff000);
                        char fileModeString[11+1];
                        
                        strmode(va_mode, fileModeString);
                        va_type = iftovt_tab[(va_type * S_IFMT) >> 12];
                        std::cout << "mode = " << fileModeString << "(" << std::hex << kea->data.mode << ", vnode type " << std::dec << ((va_type < VTYPE_MAX) ? vtypeNames[va_type] : "?") << ")" << std::endl;
                        break;
                    }
                    default:
                        std::cout << "unknown" << std::endl;
                        break;
                }
                kea = (kfs_event_arg_t *) ((char *)kea + eoff); // next
            } // for each argument
        } // for each event
    } // forever
    
    close(cloned_fsed);
    exit(0);
}
