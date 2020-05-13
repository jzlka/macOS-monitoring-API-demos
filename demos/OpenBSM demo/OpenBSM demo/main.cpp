//
//  main.cpp
//  OpenBSM demo
//
//  Created by Jozef on 13/05/2020.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//
// Source: https://github.com/meliot/filewatcher

#include <iostream>
#include <atomic>
#include <bsm/libbsm.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <sys/syslimits.h>

#include <stdio.h>
#include <stdlib.h>
#include <security/audit/audit_ioctl.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <libproc.h>
#include <pwd.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

std::atomic<bool> g_shouldStop {false};

void signalHandler(int signum)
{
    // Not safe, but whatever
    std::cerr << "Interrupt signal (" << signum << ") received, exiting." << std::endl;
    g_shouldStop = true;
}


FILE *initPipe()
{
    // Open the device
    FILE* auditFile = fopen("/dev/auditpipe", "r");
    
    if (geteuid())
        std::cerr << "Opening /dev/auditpipe requires root permissions\n";

    if (auditFile == nullptr) {
        std::cerr << "Could not open the device\n";
        exit(1);
    }
    int auditFileDescriptor = fileno(auditFile);
    
    
    
    // Configure the audit pipe
    int mode = AUDITPIPE_PRESELECT_MODE_LOCAL;
    int ioctlReturn = ioctl(auditFileDescriptor,
                            AUDITPIPE_SET_PRESELECT_MODE,
                            &mode);
    if (ioctlReturn == -1) {
        std::cerr << "Unable to set the audit pipe mode to local.\n";
        perror("Error ");
        return nullptr;
    }

    int queueLength;
    ioctlReturn = ioctl(auditFileDescriptor,
                        AUDITPIPE_GET_QLIMIT_MAX,
                        &queueLength);
    if (ioctlReturn == -1) {
        std::cerr << "Unable to get the maximum queue length of the audit pipe.\n";
        perror("Error ");
        return nullptr;
    }

    ioctlReturn = ioctl(auditFileDescriptor,
                        AUDITPIPE_SET_QLIMIT,
                        &queueLength);
    if (ioctlReturn == -1) {
        std::cerr << "Unable to set the queue length of the audit pipe.\n";
        perror("Error ");
        return nullptr;
    }

    // According with /etc/security/audit_class
    u_int attributableEventsMask =
            //0x00000000 | // Invalid Class (no)
            0x00000001 | // File read (fr)
            0x00000002 | // File write (fw)
            0x00000004 | // File attribute access (fa)
            0x00000008 | // File attribute modify (fm)
            0x00000010 | // File create (fc)
            0x00000020 | // File delete (fd)
            0x00000040 ;     // File close (cl)
            //0x00000080 | // Process (pc)
            //0x00000100 | // Network (nt)
            //0x00000200 | // IPC (ip)
            //0x00000400 | // Non attributable (na)
            //0x00000800 | // Administrative (ad)
            //0x00001000 | // Login/Logout (lo)
            //0x00002000 | // Authentication and authorization (aa)
            //0x00004000 | // Application (ap)
            //0x20000000 | // ioctl (io)
            //0x40000000 | // exec (ex)
            //0x80000000 | // Miscellaneous (ot)
            //0xffffffff ; // All flags set (all)
    ioctlReturn = ioctl(auditFileDescriptor,
                        AUDITPIPE_SET_PRESELECT_FLAGS,
                        &attributableEventsMask);
    if (ioctlReturn == -1) {
        std::cerr << "Unable to set the attributable events preselection mask.\n";
        perror("Error ");
        return nullptr;
    }

    u_int nonAttributableEventsMask = attributableEventsMask;
    ioctlReturn = ioctl(auditFileDescriptor,
                        AUDITPIPE_SET_PRESELECT_NAFLAGS,
                        &nonAttributableEventsMask);
    if (ioctlReturn == -1) {
        std::cerr << "Unable to set the non-attributable events preselection mask.\n";
        perror("Error ");
        return nullptr;
    }
    
    return auditFile;
}

void readPrintToken(FILE* auditFile)
{
    u_char* buffer;
    int recordLength = au_read_rec(auditFile, &buffer);
    if (recordLength == -1)
        return;

    int recordBalance = recordLength;
    int processedLength = 0;
    tokenstr_t token;

    while (recordBalance) {
        // Extract a token from the record
        int fetchToken = au_fetch_tok(&token, buffer + processedLength, recordBalance);
        if (fetchToken == -1) {
            std::cerr << "Error fetching token.\n";
            break;
        }
        
        au_print_flags_tok(stdout, &token, ":", AU_OFLAG_XML);
        processedLength += token.len;
        recordBalance -= token.len;
    }
    free(buffer);
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
    
    const char* demoName = "OpenBSM";
    const std::string demoPath = "/tmp/" + std::string(demoName) + "-demo";
    
    std::cout << "(" << demoName << ") Hello, World!\n";
    std::cout << "Point of interest: " << "All the events!" << std::endl << std::endl;
    
    FILE *auditFile = initPipe();
    
    while(!g_shouldStop)
        readPrintToken(auditFile);
        
    fclose(auditFile);
    return EXIT_SUCCESS;
}
