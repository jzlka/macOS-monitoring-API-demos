//
//  main.mm
//  ESF demo
//
//  Created by Jozef on 15/05/2020.
//  Copyright © 2020 Jozef Zuzelka. All rights reserved.
//
// Source: https://gist.github.com/Omar-Ikram/8e6721d8e83a3da69b31d4c2612a68ba/


#include <algorithm>
#include <bsm/libbsm.h>
#include <EndpointSecurity/EndpointSecurity.h>
#include <iostream>
#include <map>
#include <signal.h>
#include <sys/fcntl.h> // FREAD, FWRITE, FFLAGS
#include <vector>
#import <Foundation/Foundation.h>

#include "../../../Common/Tools/Tools.hpp"
#include "../../../Common/Tools/Tools-ES.hpp"

// From <Kernel/sys/fcntl.h>
/* convert from open() flags to/from fflags; convert O_RD/WR to FREAD/FWRITE */
#define FFLAGS(oflags)  ((oflags) + 1)
#define OFLAGS(fflags)  ((fflags) - 1)

es_client_t *g_client = nullptr;
std::vector<const std::string> g_blockedPaths; // thread safe for reading

const inline static es_event_type_t g_eventsOfInterest[] = {
    // Process
    ES_EVENT_TYPE_AUTH_EXEC,
    ES_EVENT_TYPE_NOTIFY_EXIT,
    ES_EVENT_TYPE_NOTIFY_FORK,
    // File System
    ES_EVENT_TYPE_NOTIFY_ACCESS,
    ES_EVENT_TYPE_AUTH_CLONE,
    ES_EVENT_TYPE_NOTIFY_CLOSE,
    ES_EVENT_TYPE_AUTH_CREATE,
    ES_EVENT_TYPE_AUTH_FILE_PROVIDER_MATERIALIZE,
    ES_EVENT_TYPE_AUTH_FILE_PROVIDER_UPDATE,
    ES_EVENT_TYPE_NOTIFY_EXCHANGEDATA,
    ES_EVENT_TYPE_AUTH_LINK,
    ES_EVENT_TYPE_AUTH_MOUNT,
    ES_EVENT_TYPE_AUTH_OPEN,
    ES_EVENT_TYPE_AUTH_READDIR,
    ES_EVENT_TYPE_AUTH_READLINK,
    ES_EVENT_TYPE_AUTH_RENAME,
    ES_EVENT_TYPE_AUTH_TRUNCATE,
    ES_EVENT_TYPE_AUTH_UNLINK,
    ES_EVENT_TYPE_NOTIFY_UNMOUNT,
    ES_EVENT_TYPE_NOTIFY_WRITE,
    // System
    ES_EVENT_TYPE_NOTIFY_IOKIT_OPEN,
    ES_EVENT_TYPE_NOTIFY_KEXTLOAD,
    ES_EVENT_TYPE_NOTIFY_KEXTUNLOAD,
};

void notify_event_handler(const es_message_t *msg);
uint32_t flags_event_handler(const es_message_t *msg);
es_auth_result_t auth_event_handler(const es_message_t *msg);

void signalHandler(int signum)
{
    if(g_client) {
        es_unsubscribe_all(g_client);
        es_delete_client(g_client);
    }
    
    // Not safe, but whatever
    std::cerr << "Interrupt signal (" << signum << ") received, exiting." << std::endl;
    exit(signum);
}


int main() {
    // No runloop, no problem
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);

    const char* demoName = "ESF";
    const std::string demoPath = "/tmp/" + std::string(demoName) + "-demo";
    
    std::cout << "(" << demoName << ") Hello, World!\n";
    std::cout << "Point of interest: " << demoPath << std::endl << std::endl;
    
    
    @autoreleasepool {
        g_blockedPaths.push_back(demoPath);
        
        // Handler blocking file operations working with demoPath and monitoring mount operations
        es_handler_block_t handler = ^(es_client_t *clt, const es_message_t *msg) {
            //std::cout << msg << std::endl;

            // Handle subscribed AUTH events:
            if (msg->action_type == ES_ACTION_TYPE_AUTH) {
                es_respond_result_t res;

                if (msg->event_type == ES_EVENT_TYPE_AUTH_OPEN) {
                    res = es_respond_flags_result(clt, msg, flags_event_handler(msg), false);
                } else {
                    res = es_respond_auth_result(clt, msg, auth_event_handler(msg), false);
                }

                if (res != ES_RESPOND_RESULT_SUCCESS)
                    std::cerr << "es_respond_auth_result: " << g_respondResultToStrMap.at(res) << std::endl;
            } else {
                notify_event_handler(msg);
            }
        };
                        
        es_new_client_result_t res = es_new_client(&g_client, handler);
        
        if (ES_NEW_CLIENT_RESULT_SUCCESS != res) {
            if(ES_NEW_CLIENT_RESULT_ERR_NOT_ENTITLED == res)
                std::cerr << "Application requires 'com.apple.developer.endpoint-security.client' entitlement\n";
            else if(ES_NEW_CLIENT_RESULT_ERR_NOT_PERMITTED == res)
                std::cerr << "Application needs to run as root (and SIP disabled).\n";
            else
                std::cerr << "es_new_client: " << res << std::endl;
            
            return EXIT_FAILURE;
        }
        
        // Cache needs to be explicitly cleared between program invocations
        es_clear_cache_result_t resCache = es_clear_cache(g_client);
        if (ES_CLEAR_CACHE_RESULT_SUCCESS != resCache) {
            std::cerr << "es_clear_cache: " << resCache << std::endl;
            return EXIT_FAILURE;
        }
        
        
        // Subscribe to the events we're interested in
        es_return_t subscribed = es_subscribe(g_client,
                                       g_eventsOfInterest,
                                       (sizeof(g_eventsOfInterest) / sizeof((g_eventsOfInterest)[0])) // Event count
                                       );
        
        if (subscribed == ES_RETURN_ERROR) {
            std::cerr << "es_subscribe: ES_RETURN_ERROR\n";
            return EXIT_FAILURE;
        }
        
        dispatch_main();
    }
    
    return EXIT_SUCCESS;
}


static const auto find_occurence = [](const std::string& str) {
    for (const auto &path : g_blockedPaths) {
        if (str.find(path) != std::string::npos) {
            std::cout << "*** Occurence found: " << str << std::endl;
            return true;
        }
    }
    return false;
};

void notify_event_handler(const es_message_t *msg)
{
    switch(msg->event_type) {
        // Process
        case ES_EVENT_TYPE_NOTIFY_EXIT:
        case ES_EVENT_TYPE_NOTIFY_FORK:
        // System
        case ES_EVENT_TYPE_NOTIFY_IOKIT_OPEN:
            break;
        case ES_EVENT_TYPE_NOTIFY_KEXTLOAD:
        case ES_EVENT_TYPE_NOTIFY_KEXTUNLOAD:
        // File System
        case ES_EVENT_TYPE_NOTIFY_UNMOUNT:
            std::cout << "NOTIFY OPERATION: " << g_eventTypeToStrMap.at(msg->event_type) << std::endl;
            std::cout << msg << std::endl;
            break;
        // File System
        case ES_EVENT_TYPE_NOTIFY_ACCESS:
        case ES_EVENT_TYPE_NOTIFY_CLOSE:
        case ES_EVENT_TYPE_NOTIFY_EXCHANGEDATA:
        case ES_EVENT_TYPE_NOTIFY_WRITE:
        {
            const std::vector<std::string> eventPaths = paths_from_event(msg);

            // Block if path is in our blocked paths list
            if (std::any_of(eventPaths.cbegin(), eventPaths.cend(), find_occurence)) {
                std::cout << "    " << (msg->action_type == ES_ACTION_TYPE_AUTH ? "BLOCKING: " : "NOTIFY: ")
                          << g_eventTypeToStrMap.at(msg->event_type) << " at "
                          << (long long) msg->mach_time << " of mach time." << std::endl;
                std::cout << msg << std::endl;
            }
            break;
        }
        default:
            std::cout << "DEFAULT: " << g_eventTypeToStrMap.at(msg->event_type) << std::endl;
            break;
    }
}

uint32_t flags_event_handler(const es_message_t *msg)
{
    uint32_t res = msg->event.open.fflag;

    switch(msg->event_type) {
        case ES_EVENT_TYPE_AUTH_OPEN:
        {
            const std::vector<std::string> eventPaths = paths_from_event(msg);

            // Block if path is in our blocked paths list
            if (std::any_of(eventPaths.cbegin(), eventPaths.cend(), find_occurence)) {
                std::cout << "    " << (msg->action_type == ES_ACTION_TYPE_AUTH ? "BLOCKING: " : "NOTIFY: ")
                          << g_eventTypeToStrMap.at(msg->event_type) << " at "
                          << (long long) msg->mach_time << " of mach time." << std::endl;
                std::cout << msg << std::endl;
                res = FFLAGS(O_RDONLY);
            }
            break;
        }
        default:
            std::cout << "DEFAULT: " << g_eventTypeToStrMap.at(msg->event_type) << std::endl;
            break;
    }

    return res;
}

// Simple handler to make AUTH (allow or block) decisions.
// Returns either an ES_AUTH_RESULT_ALLOW or ES_AUTH_RESULT_DENY.
es_auth_result_t auth_event_handler(const es_message_t *msg)
{
    switch(msg->event_type) {
        // Process
        case ES_EVENT_TYPE_AUTH_EXEC:
            break;
        // System
        // File System
        case ES_EVENT_TYPE_AUTH_MOUNT:
            std::cout << "ALLOWING OPERATION: " << g_eventTypeToStrMap.at(msg->event_type) << std::endl;
            std::cout << msg << std::endl;
            break;
        // File System
        case ES_EVENT_TYPE_AUTH_CREATE:
        case ES_EVENT_TYPE_AUTH_CLONE:
        case ES_EVENT_TYPE_AUTH_FILE_PROVIDER_MATERIALIZE:
        case ES_EVENT_TYPE_AUTH_FILE_PROVIDER_UPDATE:
        case ES_EVENT_TYPE_AUTH_LINK:
        case ES_EVENT_TYPE_AUTH_READDIR:
        case ES_EVENT_TYPE_AUTH_READLINK:
        case ES_EVENT_TYPE_AUTH_RENAME:
        case ES_EVENT_TYPE_AUTH_TRUNCATE:
        case ES_EVENT_TYPE_AUTH_UNLINK:
        {
            const std::vector<std::string> eventPaths = paths_from_event(msg);

            // Block if path is in our blocked paths list
            if (std::any_of(eventPaths.cbegin(), eventPaths.cend(), find_occurence)) {
                std::cout << "    " << (msg->action_type == ES_ACTION_TYPE_AUTH ? "BLOCKING: " : "NOTIFY: ")
                          << g_eventTypeToStrMap.at(msg->event_type) << " at "
                          << (long long) msg->mach_time << " of mach time." << std::endl;
                std::cout << msg << std::endl;

                return ES_AUTH_RESULT_DENY;
            }
            break;
        }
        default:
            std::cout << "DEFAULT: " << g_eventTypeToStrMap.at(msg->event_type) << std::endl;
            break;
    }

    return ES_AUTH_RESULT_ALLOW;
}
