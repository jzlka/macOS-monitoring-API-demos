//
//  Tools-ES.mm
//  
//
//  Created by Jozef on 18/05/2020.
//
// TODO: NOT ALL ES TYPES ARE SUPPORTED YET !!!
// Inspired by: https://gist.github.com/Omar-Ikram/8e6721d8e83a3da69b31d4c2612a68ba/

#include <iostream>
#include <map>
#include <Foundation/Foundation.h>
#include <bsm/libbsm.h>
#include <EndpointSecurity/EndpointSecurity.h>
#include <mach/mach_time.h>

#include "Tools-ES.hpp"

const inline std::map<es_event_type_t, const std::string> g_eventTypeToStrMap = {
    // Process
    {ES_EVENT_TYPE_AUTH_EXEC, "ES_EVENT_TYPE_AUTH_EXEC"},
    {ES_EVENT_TYPE_NOTIFY_EXIT, "ES_EVENT_TYPE_NOTIFY_EXIT"},
    {ES_EVENT_TYPE_NOTIFY_FORK, "ES_EVENT_TYPE_NOTIFY_FORK"},
    // File System
    {ES_EVENT_TYPE_NOTIFY_ACCESS, "ES_EVENT_TYPE_NOTIFY_ACCESS"},
    {ES_EVENT_TYPE_AUTH_CLONE, "ES_EVENT_TYPE_AUTH_CLONE"},
    {ES_EVENT_TYPE_NOTIFY_CLOSE, "ES_EVENT_TYPE_NOTIFY_CLOSE"},
    {ES_EVENT_TYPE_AUTH_CREATE, "ES_EVENT_TYPE_AUTH_CREATE"},
    {ES_EVENT_TYPE_AUTH_FILE_PROVIDER_MATERIALIZE, "ES_EVENT_TYPE_AUTH_FILE_PROVIDER_MATERIALIZE"},
    {ES_EVENT_TYPE_AUTH_FILE_PROVIDER_UPDATE, "ES_EVENT_TYPE_AUTH_FILE_PROVIDER_UPDATE"},
    {ES_EVENT_TYPE_NOTIFY_EXCHANGEDATA, "ES_EVENT_TYPE_NOTIFY_EXCHANGEDATA"},
    {ES_EVENT_TYPE_AUTH_LINK, "ES_EVENT_TYPE_AUTH_LINK"},
    {ES_EVENT_TYPE_AUTH_MOUNT, "ES_EVENT_TYPE_AUTH_MOUNT"},
    {ES_EVENT_TYPE_AUTH_OPEN, "ES_EVENT_TYPE_AUTH_OPEN"},
    {ES_EVENT_TYPE_AUTH_READDIR, "ES_EVENT_TYPE_AUTH_READDIR"},
    {ES_EVENT_TYPE_AUTH_READLINK, "ES_EVENT_TYPE_AUTH_READLINK"},
    {ES_EVENT_TYPE_AUTH_RENAME, "ES_EVENT_TYPE_AUTH_RENAME"},
    {ES_EVENT_TYPE_AUTH_TRUNCATE, "ES_EVENT_TYPE_AUTH_TRUNCATE"},
    {ES_EVENT_TYPE_AUTH_UNLINK, "ES_EVENT_TYPE_AUTH_UNLINK"},
    {ES_EVENT_TYPE_NOTIFY_UNMOUNT, "ES_EVENT_TYPE_NOTIFY_UNMOUNT"},
    {ES_EVENT_TYPE_NOTIFY_WRITE, "ES_EVENT_TYPE_NOTIFY_WRITE"},
    // System
    {ES_EVENT_TYPE_NOTIFY_IOKIT_OPEN, "ES_EVENT_TYPE_NOTIFY_IOKIT_OPEN"},
    {ES_EVENT_TYPE_NOTIFY_KEXTLOAD, "ES_EVENT_TYPE_NOTIFY_KEXTLOAD"},
    {ES_EVENT_TYPE_NOTIFY_KEXTUNLOAD, "ES_EVENT_TYPE_NOTIFY_KEXTUNLOAD"},
    // !!!: Default --> throws an exception
};

const inline std::map<es_respond_result_t, const std::string> g_respondResultToStrMap = {
    {ES_RESPOND_RESULT_SUCCESS, "ES_RESPOND_RESULT_SUCCESS"},
    ///One or more invalid arguments were provided
    {ES_RESPOND_RESULT_ERR_INVALID_ARGUMENT, "ES_RESPOND_RESULT_ERR_INVALID_ARGUMENT"},
    ///Communication with the ES subsystem failed
    {ES_RESPOND_RESULT_ERR_INTERNAL, "ES_RESPOND_RESULT_ERR_INTERNAL"},
    ///The message being responded to could not be found
    {ES_RESPOND_RESULT_NOT_FOUND, "ES_RESPOND_RESULT_NOT_FOUND"},
    ///The provided message has been responded to more than once
    {ES_RESPOND_RESULT_ERR_DUPLICATE_RESPONSE, "ES_RESPOND_RESULT_ERR_DUPLICATE_RESPONSE"},
    ///Either an inappropriate response API was used for the event type (ensure using proper
    ///es_respond_auth_result or es_respond_flags_result function) or the event is notification only.
    {ES_RESPOND_RESULT_ERR_EVENT_TYPE, "ES_RESPOND_RESULT_ERR_EVENT_TYPE"},
};

const inline static std::map<es_destination_type_t, const std::string> g_destinationTypeToStrMap = {
    {ES_DESTINATION_TYPE_EXISTING_FILE, "ES_DESTINATION_TYPE_EXISTING_FILE"},
    {ES_DESTINATION_TYPE_NEW_PATH, "ES_DESTINATION_TYPE_EXISTING_FILE"},
};


// MARK: - Custom Casts
@implementation NSString (alternativeConstructorsEs)

+(NSString*)stringFromEsString:(es_string_token_t const &)esString
{
    NSString *res = @"";

    if (esString.data && esString.length > 0) {
        // es_string_token.data is a pointer to a null-terminated string
        res = [NSString stringWithUTF8String:esString.data];
    }

    return res;
}

@end

std::string to_string(const es_string_token_t &esString)
{
    if (esString.data == nullptr || esString.length <= 0)
        return "(null)";
    
    return std::string(esString.data, esString.length);
}


// MARK: - Endpoint Security Logging

// MARK: Process Events
std::ostream & operator << (std::ostream &out, const es_event_exec_t &event)
{
    out << "event.exec.target:\n" << event.target;

    // Log program arguments
    const uint32_t argc = es_exec_arg_count(&event);
    out << std::endl << "event.exec.arg_count: " << argc;

    // Extract each argument and log it out
    for (uint32_t i = 0; i < argc; i++)
        out << std::endl << "arg " << i << ": " << es_exec_arg(&event, i);

    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_exit_t &event)
{
    out << "event.exit.stat: " << event.stat;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_fork_t &event)
{
    out << "event.fork.child:\n" << event.child;
    return out;
}

// MARK: File System Events
std::ostream & operator << (std::ostream &out, const es_event_access_t &event)
{
    out << "event.access.mode: " << event.mode;
    out << std::endl << "event.access.target:\n" << event.target;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_clone_t &event)
{
    out << "event.clone.source:\n" << event.source;
    out << std::endl << "event.clone.target_dir:\n" << event.target_dir;
    out << std::endl << "event.clone.target_name: " << event.target_name;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_close_t &event)
{
    out << "event.close.modified: " << event.modified;
    out << std::endl << "event.close.target: \n" << event.target;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_create_t &event)
{
    out << std::endl << "event.create.destination_type: " << g_destinationTypeToStrMap.at(event.destination_type);
    if (event.destination_type == ES_DESTINATION_TYPE_EXISTING_FILE) {
        out << std::endl << "event.create.destination.existing_file:\n" << event.destination.existing_file;
    } else {
        out << std::endl << "event.create.destination.new_path.dir:\n" << event.destination.new_path.dir;
        out << std::endl << "event.create.destination.new_path.filename: " << event.destination.new_path.filename;
    }
    return out;
    // TODO: acl
}

std::ostream & operator << (std::ostream &out, const es_event_file_provider_materialize_t &event)
{
    out << "event.file_provider_materialize.instigator:\n" << event.instigator;
    out << std::endl << "event.file_provider_materialize.source:\n" << event.source;
    out << std::endl << "event.file_provider_materialize.target:\n" << event.target;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_file_provider_update_t &event)
{
    out << std::endl << "event.file_provider_update.source:\n" << event.source;
    out << std::endl << "event.file_provider_update.target_path: " << event.target_path;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_exchangedata_t &event)
{
    out << "event.exchangedata.file1:\n" << event.file1;
    out << std::endl << "event.exchangedata.file2:\n" << event.file2;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_link_t &event)
{
    out << "event.link.source:\n" << event.source;
    out << std::endl << "event.link.target_dir:\n" << event.target_dir;
    out << std::endl << "event.link.target_filename: " << event.target_filename;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_mount_t &event)
{
    out << "event.mount.statfs:\n" << event.statfs;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_open_t &event)
{
    out << "event.open.fflag: " << std::hex << event.fflag << std::dec;
    out << std::endl << event.file;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_readdir_t &event)
{
    out << "event.readdir.target:\n" << event.target;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_readlink_t &event)
{
    out << "event.readlink.source:\n" << event.source;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_rename_t &event)
{
    out << "event.rename.source:\n" << event.source;
    out << std::endl << "event.rename.destination_type: " << g_destinationTypeToStrMap.at(event.destination_type);
    if (event.destination_type == ES_DESTINATION_TYPE_EXISTING_FILE) {
        out << std::endl << "event.rename.destination.existing_file:\n" << event.destination.existing_file;
    } else {
        out << std::endl << "event.rename.destination.new_path.dir:\n" << event.destination.new_path.dir;
        out << std::endl << "event.rename.destination.new_path.filename: " << event.destination.new_path.filename;
    }
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_truncate_t &event)
{
    out << "event.truncate.target:\n" << event.target;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_unlink_t &event)
{
    out << "event.unlink.target:\n" << event.target;
    out << std::endl << "event.unlink.parent_dir" << event.parent_dir;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_unmount_t &event)
{
    out << "event.unmount.statfs:\n" << event.statfs;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_write_t &event)
{
    out << "event.write.target:\n" << event.target;
    return out;
}

// MARK: System Events
std::ostream & operator << (std::ostream &out, const es_event_iokit_open_t &event)
{
    out << "event.iokit_open.user_client_type: " << event.user_client_type;
    out << std::endl << "event.iokit_open.user_client_class: " << event.user_client_class;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_kextload_t &event)
{
    out << "event.kextload.identifier: " << event.identifier;
    return out;
}

std::ostream & operator << (std::ostream &out, const es_event_kextunload_t &event)
{
    out << "event.kextunload.identifier: " << event.identifier;
    return out;
}

// MARK: ES Types
std::ostream & operator << (std::ostream &out, const es_message_t *msg)
{
    out << "--- EVENT MESSAGE ----";
    out << std::endl << "event_type: " << g_eventTypeToStrMap.at(msg->event_type) << " (" << msg->event_type << ")";
     // Note: Message structure could change in future versions
    out << std::endl << "version: " << msg->version;
    out << std::endl << "time: " << (long long) msg->time.tv_sec << "." << msg->time.tv_nsec;
    out << std::endl << "mach_time: " << (long long) msg->mach_time;

    // It's very important that the message is processed within the deadline:
    // https://developer.apple.com/documentation/endpointsecurity/es_message_t/3334985-deadline
    out << std::endl << "deadline: " << (long long) msg->deadline;

    uint64_t deadlineInterval = msg->deadline;
    if(deadlineInterval > 0)
        deadlineInterval -= msg->mach_time;

    out << std::endl << "deadline interval: " << (long long) deadlineInterval << " (" << (int) (deadlineInterval / 1.0e9) << " seconds)";

    out << std::endl << "action_type: " << ((msg->action_type == ES_ACTION_TYPE_AUTH) ? "Auth" : "Notify");
    out << std::endl << "- process -\n" << msg->process;
    out << std::endl << "- event -\n";

     // Type specific logging
    switch(msg->event_type) {
        case ES_EVENT_TYPE_AUTH_EXEC:
            out << msg->event.exec << std::endl;
            break;
        case ES_EVENT_TYPE_NOTIFY_EXIT:
            out << msg->event.exit << std::endl;
            break;
        case ES_EVENT_TYPE_AUTH_OPEN:
            out << msg->event.open;
            break;
        case ES_EVENT_TYPE_AUTH_RENAME:
            out << msg->event.rename;
            break;
        case ES_EVENT_TYPE_AUTH_UNLINK:
            out << msg->event.unlink;
            break;
        case ES_EVENT_TYPE_NOTIFY_EXCHANGEDATA:
            out << msg->event.exchangedata;
            break;
        case ES_EVENT_TYPE_NOTIFY_WRITE:
            out << msg->event.write;
            break;
        case ES_EVENT_TYPE_AUTH_TRUNCATE:
            out << msg->event.truncate;
            break;
        case ES_EVENT_TYPE_AUTH_CREATE:
            out << msg->event.create;
            break;
        case ES_EVENT_TYPE_AUTH_MOUNT:
            out << msg->event.mount;
            break;
        case ES_EVENT_TYPE_NOTIFY_UNMOUNT:
            out << msg->event.unmount;
            break;
        case ES_EVENT_TYPE_NOTIFY_KEXTLOAD:
            out << msg->event.kextload;
            break;
        case ES_EVENT_TYPE_NOTIFY_KEXTUNLOAD:
            out << msg->event.kextunload;
            break;
        case ES_EVENT_TYPE_LAST:
        default:
            out << std::endl << "Printing not implemented yet: " << g_eventTypeToStrMap.at(msg->event_type);
            break;
    }
    return out;
}

std::ostream & operator << (std::ostream &out, const es_string_token_t &esString)
{
    out << (esString.data ? std::string(esString.data, esString.length) : "(null)");
    return out;
}

std::ostream & operator << (std::ostream &out, const es_file_t *file)
{
    if(!file)
        return out;

    out << " file.path: " << file->path;
    out << std::endl << " file.path_truncated: " << file->path_truncated;
    out << std::endl << " file.stats: " << "TO BE DONE";
    return out;
}

std::ostream & operator << (std::ostream &out, const es_statfs_t *stats)
{
    if(!stats)
        return out;
    out << " statfs: " << "TO BE DONE";
    return out;
}

std::ostream & operator << (std::ostream &out, const es_process_t * const proc)
{
    if(!proc)
        return out;

    out << "  proc.pid: " << audit_token_to_pid(proc->audit_token) << std::endl;
    out << "  proc.ppid: " << proc->ppid << std::endl;
    out << "  proc.original_ppid: " << proc->original_ppid << std::endl;
    out << "  proc.ruid: " << audit_token_to_ruid(proc->audit_token) << std::endl;
    out << "  proc.euid: " << audit_token_to_euid(proc->audit_token) << std::endl;
    out << "  proc.rgid: " << audit_token_to_rgid(proc->audit_token) << std::endl;
    out << "  proc.egid: " << audit_token_to_egid(proc->audit_token) << std::endl;
    out << "  proc.group_id: " << proc->group_id << std::endl;
    out << "  proc.session_id: " << proc->session_id << std::endl;
    out << "  proc.codesigning_flags: " << std::hex << proc->codesigning_flags << std::dec << std::endl;
    out << "  proc.is_platform_binary: %s" << proc->is_platform_binary << std::endl;
    out << "  proc.is_es_client: " << proc->is_es_client << std::endl;
    out << "  proc.signing_id: " << proc->signing_id << std::endl;
    out << "  proc.team_id: " << proc->team_id << std::endl;

    // proc.cdhash
    NSMutableString *hash = [NSMutableString string];
    for(uint32_t i = 0; i < CS_CDHASH_LEN; i++) {
        [hash appendFormat:@"%x", proc->cdhash[i]];
    }
    out << "  proc.cdhash: " << hash << std::endl;
    out << "  proc.executable.path: " << proc->executable;

    return out;
}
