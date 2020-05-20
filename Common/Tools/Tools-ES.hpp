//
//  Tools-ES.hpp
//  
//
//  Created by Jozef on 18/05/2020.
//

#ifndef Tools_ES_hpp
#define Tools_ES_hpp

extern const std::map<es_event_type_t, const std::string> g_eventTypeToStrMap;
extern const std::map<es_respond_result_t, const std::string> g_respondResultToStrMap;

// MARK: - Custom Casts
@interface NSString (alternativeConstructorsEs)
+ (NSString*)stringFromEsString:(es_string_token_t const &)esString;
@end

std::string to_string(const es_string_token_t &esString);
std::vector<const std::string> paths_from_file_event(const es_message_t * const msg);

// MARK: - Endpoint Security Logging
// MARK: Process Events
std::ostream & operator << (std::ostream &out, const es_event_exec_t &event);
std::ostream & operator << (std::ostream &out, const es_event_exit_t &event);
std::ostream & operator << (std::ostream &out, const es_event_fork_t &event);
// MARK: File System Events
std::ostream & operator << (std::ostream &out, const es_event_access_t &event);
std::ostream & operator << (std::ostream &out, const es_event_clone_t &event);
std::ostream & operator << (std::ostream &out, const es_event_close_t &event);
std::ostream & operator << (std::ostream &out, const es_event_create_t &event);
std::ostream & operator << (std::ostream &out, const es_event_file_provider_materialize_t &event);
std::ostream & operator << (std::ostream &out, const es_event_file_provider_update_t &event);
std::ostream & operator << (std::ostream &out, const es_event_exchangedata_t &event);
std::ostream & operator << (std::ostream &out, const es_event_link_t &event);
std::ostream & operator << (std::ostream &out, const es_event_mount_t &event);
std::ostream & operator << (std::ostream &out, const es_event_open_t &event);
std::ostream & operator << (std::ostream &out, const es_event_readdir_t &event);
std::ostream & operator << (std::ostream &out, const es_event_readlink_t &event);
std::ostream & operator << (std::ostream &out, const es_event_rename_t &event);
std::ostream & operator << (std::ostream &out, const es_event_truncate_t &event);
std::ostream & operator << (std::ostream &out, const es_event_unlink_t &event);
std::ostream & operator << (std::ostream &out, const es_event_unmount_t &event);
std::ostream & operator << (std::ostream &out, const es_event_write_t &event);
// MARK: System Events
std::ostream & operator << (std::ostream &out, const es_event_iokit_open_t &event);
std::ostream & operator << (std::ostream &out, const es_event_kextload_t &event);
std::ostream & operator << (std::ostream &out, const es_event_kextunload_t &event);
// MARK: ES Types
std::ostream & operator << (std::ostream &out, const es_message_t * const msg);
std::ostream & operator << (std::ostream &out, const es_string_token_t &esString);
std::ostream & operator << (std::ostream &out, const es_file_t * const file);
std::ostream & operator << (std::ostream &out, const es_statfs_t * const stats);
std::ostream & operator << (std::ostream &out, const es_process_t * const proc);

#endif /* Tools_ES_hpp */
