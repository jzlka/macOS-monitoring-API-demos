//
//  kauth_demo.c
//  kauth demo
//
//  Created by Jozef on 29/05/2020.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//
// Sources:
// https://developer.apple.com/library/archive/samplecode/KauthORama/Introduction/Intro.html

// Some kernel headers get grumpy when compiled with the latest compiler warnings, so
// disable those warnings while including those headers.

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wsign-conversion"

#include <mach/mach_types.h>

#include <kern/assert.h>
#include <mach/mach_types.h>
#include <libkern/libkern.h>
#include <libkern/OSMalloc.h>
#include <sys/sysctl.h>
#include <sys/kauth.h>
#include <sys/vnode.h>

#pragma clang diagnostic pop


#pragma mark ***** Global Resources
// These declarations are required to allocate memory and create locks.
// They're created when we start and destroyed when we stop.
static OSMallocTag  gMallocTag = NULL;

#define BUNDLE_NAME "com_jzlka_kauth_demo"
#define BUNDLE_ID "com.jzlka.kauth-demo"
const char* g_demoName = "kauth";
const char* g_demoPath = "/tmp/kauth-demo";



#pragma mark ***** Vnode Utilities

// I've pulled these vnode utility routines out of the VnodeScopeListener to make it
// easier to understand.

// VnodeActionInfo describes one of the action bits in the vnode scope's action
// field.

struct VnodeActionInfo {
    kauth_action_t      fMask;                  // only one bit should be set
    const char *        fOpNameFile;            // descriptive name of the bit for files
    const char *        fOpNameDir;             // descriptive name of the bit for directories
                                                // NULL implies equivalent to fOpNameFile
};
typedef struct VnodeActionInfo VnodeActionInfo;

// Some evil macros (aren't they all) to make it easier to initialise kVnodeActionInfo.

#define VNODE_ACTION(action)                        { KAUTH_VNODE_ ## action,     #action,     NULL       }
#define VNODE_ACTION_FILEDIR(actionFile, actionDir) { KAUTH_VNODE_ ## actionFile, #actionFile, #actionDir }

// kVnodeActionInfo is a table of all the known action bits and their human readable names.

static const VnodeActionInfo kVnodeActionInfo[] = {
    VNODE_ACTION_FILEDIR(READ_DATA,   LIST_DIRECTORY),
    VNODE_ACTION_FILEDIR(WRITE_DATA,  ADD_FILE),
    VNODE_ACTION_FILEDIR(EXECUTE,     SEARCH),
    VNODE_ACTION(DELETE),
    VNODE_ACTION_FILEDIR(APPEND_DATA, ADD_SUBDIRECTORY),
    VNODE_ACTION(DELETE_CHILD),
    VNODE_ACTION(READ_ATTRIBUTES),
    VNODE_ACTION(WRITE_ATTRIBUTES),
    VNODE_ACTION(READ_EXTATTRIBUTES),
    VNODE_ACTION(WRITE_EXTATTRIBUTES),
    VNODE_ACTION(READ_SECURITY),
    VNODE_ACTION(WRITE_SECURITY),
    VNODE_ACTION(TAKE_OWNERSHIP),
    VNODE_ACTION(SYNCHRONIZE),
    VNODE_ACTION(LINKTARGET),
    VNODE_ACTION(CHECKIMMUTABLE),
    VNODE_ACTION(ACCESS),
    VNODE_ACTION(NOIMMUTABLE)
};

#define kVnodeActionInfoCount (sizeof(kVnodeActionInfo) / sizeof(*kVnodeActionInfo))

enum {
    kActionStringMaxLength = 16384
};

static int CreateVnodeActionString(
    kauth_action_t  action,
    boolean_t       isDir,
    char **         actionStrPtr,
    size_t *        actionStrBufSizePtr
)
    // Creates a human readable description of a vnode action bitmap.
    // action is the bitmap.  isDir is true if the action relates to a
    // directory, and false otherwise.  This allows the action name to
    // be context sensitive (KAUTH_VNODE_EXECUTE vs KAUTH_VNODE_SEARCH).
    // actionStrPtr is a place to store the allocated string pointer.
    // The caller is responsible for freeing this memory using OSFree.
    // actionStrBufSizePtr is a place to store the size of the resulting
    // allocation (because the annoying kernel memory allocator requires
    // you to provide the size when you free).
{
    int             err;
    enum { kCalcLen, kCreateString } pass;
    kauth_action_t  actionsLeft;
    unsigned int    infoIndex;
    size_t          actionStrLen = 0;
    size_t          actionStrSize;
    char *          actionStr;

    assert( actionStrPtr != NULL);
    assert(*actionStrPtr != NULL);
    assert( actionStrBufSizePtr != NULL);

    err = 0;

    actionStr = NULL;
    actionStrSize = 0;

    // A two pass algorithm.  In the first pass, actionStr is NULL and we just
    // calculate actionStrLen; at the end of the first pass we actually allocate
    // actionStr.  In the second pass, actionStr is not NULL and we actually
    // initialise the string in that buffer.

    for (pass = kCalcLen; pass <= kCreateString; pass++) {
        actionsLeft = action;

        // Process action bits that are described in kVnodeActionInfo.

        infoIndex = 0;
        actionStrLen = 0;
        while ( (actionsLeft != 0) && (infoIndex < kVnodeActionInfoCount) ) {
            if ( actionsLeft & kVnodeActionInfo[infoIndex].fMask ) {
                const char * thisStr;
                size_t       thisStrLen;

                // Increment the length of the acion string by the action name.

                if ( isDir && (kVnodeActionInfo[infoIndex].fOpNameDir != NULL) ) {
                    thisStr = kVnodeActionInfo[infoIndex].fOpNameDir;
                } else {
                    thisStr = kVnodeActionInfo[infoIndex].fOpNameFile;
                }
                thisStrLen = strlen(thisStr);

                if (actionStr != NULL) {
                    memcpy(&actionStr[actionStrLen], thisStr, thisStrLen);
                }
                actionStrLen += thisStrLen;

                // Now clear the bit in actionsLeft, indicating that we've
                // processed this one.

                actionsLeft &= ~kVnodeActionInfo[infoIndex].fMask;

                // If there's any actions left, account for the intervening "|".

                if (actionsLeft != 0) {
                    if (actionStr != NULL) {
                        actionStr[actionStrLen] = '|';
                    }
                    actionStrLen += 1;
                }
            }
            infoIndex += 1;
        }

        // Now include any remaining actions as a hex number.

        if (actionsLeft != 0) {
            if (actionStr != NULL) {
                // This will write 11 bytes (10 bytes of string plus a null
                // char), but that's OK because we know that we allocated
                // space for the null.

                snprintf(&actionStr[actionStrLen], actionStrSize - actionStrLen, "0x%08x", actionsLeft);
            }
            actionStrLen += 10;         // strlen("0x") + 8 chars of hex
        }

        // If we're at the end of the first pass, allocate actionStr
        // based on the size we just calculated.  Remember that actionStrLen
        // is a string length, so we have to allocate an extra character to
        // account for the null terminator.  If we're at the end of the
        // second pass, just place the null terminator.

        if (pass == kCalcLen) {
            if (actionStrLen > kActionStringMaxLength) {
                err = ENOBUFS;
            } else {
                actionStrSize = actionStrLen + 1;
                actionStr = OSMalloc( (uint32_t) actionStrSize, gMallocTag);       // The cast won't truncate because of kActionStringMaxLength check.
                if (actionStr == NULL) {
                    err = ENOMEM;
                }
            }
        } else {
            actionStr[actionStrLen] = 0;
        }

        if (err != 0) {
            break;
        }
    }

    // Clean up.

    *actionStrPtr        = actionStr;
    *actionStrBufSizePtr = actionStrLen + 1;

    assert( (err == 0) == (*actionStrPtr != NULL) );

    return err;
}

static int CreateVnodePath(vnode_t vp, char **vpPathPtr)
    // Creates a full path for a vnode.  vp may be NULL, in which
    // case the returned path is NULL (that is, no memory is allocated).
    // vpPathPtr is a place to store the allocated path buffer.
    // The caller is responsible for freeing this memory using OSFree
    // (the size is always MAXPATHLEN).
{
    int             err;
    int             pathLen;

    assert( vpPathPtr != NULL);
    assert(*vpPathPtr == NULL);

    err = 0;
    if (vp != NULL) {
        *vpPathPtr = OSMalloc(MAXPATHLEN, gMallocTag);
        if (*vpPathPtr == NULL) {
            err = ENOMEM;
        }
        if (err == 0) {
            pathLen = MAXPATHLEN;
            err = vn_getpath(vp, *vpPathPtr, &pathLen);
        }
    }

    return err;
}



// A Kauth listener that's called to authorize an action in the vnode
// scope (KAUTH_SCOPE_PROCESS).  See the Kauth documentation for a description
// of the parameters.  In this case, we just dump out the parameters to the
// operation and return KAUTH_RESULT_DEFER, allowing the other listeners
// to decide whether the operation is allowed or not.
static int
VnodeScopeListener(kauth_cred_t credential, void *idata, kauth_action_t action, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
    #pragma unused(credential)
    #pragma unused(idata)
    #pragma unused(arg3)
    int err;
    boolean_t isDir;
    size_t actionStrBufSize = 0;

    vfs_context_t context = (vfs_context_t) arg0;
    vnode_t vp            = (vnode_t) arg1;
    vnode_t dvp           = (vnode_t) arg2;

    char *vpPath    = NULL;
    char *dvpPath   = NULL;
    char *actionStr = NULL;

    // Convert the vnode, if any, to a path.
    err = CreateVnodePath(vp, &vpPath);

    // Convert the parent directory vnode, if any, to a path.
    if (err == 0) {
        err = CreateVnodePath(dvp, &dvpPath);
    }

    // Create actionStr as a human readable description of action.
    if (err == 0) {
        if (vp != NULL) {
            isDir = (vnode_vtype(vp) == VDIR);
        } else {
            isDir = FALSE;
        }
        err = CreateVnodeActionString(action, isDir, &actionStr, &actionStrBufSize);
    }

    // Tell the user about this request.  Note that we filter requests
    // based on gPrefix.  If gPrefix is set, only requests where one
    // of the paths is prefixed by gPrefix will be printed.

    if (err == 0) {
        if ((vpPath != NULL && strstr(vpPath, g_demoPath))
            || (dvpPath != NULL && strstr(dvpPath, g_demoPath))
           ) {
            printf(
                "%s scope=" KAUTH_SCOPE_VNODE ", action=%s, uid=%ld, vp=%s, dvp=%s\n",
                g_demoName,
                actionStr,
                (long) kauth_cred_getuid(vfs_context_ucred(context)),
                (vpPath  != NULL) ? vpPath : "<null>",
                (dvpPath != NULL) ? dvpPath : "<null>"
            );
        }
    } else {
        printf("%s.VnodeScopeListener: Error %d.\n", g_demoName, err);
    }

    // Clean up.
    if (actionStr != NULL) {
        // In the following, the cast can't truncate because actionStrBufSize is returned by
        // CreateVnodeActionString, which enforces a bound of kActionStringMaxLength.
        OSFree(actionStr, (uint32_t) actionStrBufSize, gMallocTag);
    }
    if (vpPath != NULL) {
        OSFree(vpPath, MAXPATHLEN, gMallocTag);
    }
    if (dvpPath != NULL) {
        OSFree(dvpPath, MAXPATHLEN, gMallocTag);
    }

    return KAUTH_RESULT_DEFER;
}

// gListener is our handle to the installed scope listener.  We need to
// keep it around so that we can remove the listener when we're done.
static kauth_listener_t gListener = NULL;

// Removes the installed scope listener, if any.
static void RemoveListener(void)
{
    // First prevent any more threads entering our listener.
    if (gListener != NULL) {
        kauth_unlisten_scope(gListener);
        gListener = NULL;
    }
}

static void
InstallListener()
{
    //kauth_authorize_action
    gListener = kauth_listen_scope(KAUTH_SCOPE_VNODE, VnodeScopeListener, NULL);
    if (gListener == NULL) {
        printf("%s.InstallListener: Could not create gListener.\n", g_demoName);
        RemoveListener();
    }
}


#pragma mark ***** Start/Stop

kern_return_t kauth_demo_start(kmod_info_t * ki, void *d);
kern_return_t kauth_demo_stop(kmod_info_t *ki, void *d);


// Called by the system to start up the kext.
kern_return_t
kauth_demo_start(kmod_info_t * ki, void *d)
{
    #pragma unused(ki)
    #pragma unused(d)

    kern_return_t   err = KERN_SUCCESS;

    gMallocTag = OSMalloc_Tagalloc("com.example.apple-samplecode.kext.KauthORama", OSMT_DEFAULT);
    if (gMallocTag == NULL) {
        err = KERN_FAILURE;
    }

    printf("(%s)_start: Hello Cruel World!\n", g_demoName);
    printf("Point of interest: %s\n", g_demoPath);

    InstallListener();
    // If we failed, shut everything down.
    if (err != KERN_SUCCESS)
        (void) kauth_demo_stop(ki, d);

    return err;
}

// Called by the system to shut down the kext.
kern_return_t
kauth_demo_stop(kmod_info_t *ki, void *d)
{
    #pragma unused(ki)
    #pragma unused(d)

    RemoveListener();

    if (gMallocTag != NULL) {
        OSMalloc_Tagfree(gMallocTag);
        gMallocTag = NULL;
    }

    // And we're done.
    printf("(%s)_stop: Goodbye Cruel World!\n", g_demoName);
    return KERN_SUCCESS;
}
