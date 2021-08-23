/**
 * @file   spider-internal.h 
 * @author Mohamed Al-Hussein
 * @date   08/21/2021
 * @brief  Internal header file for Spider.
 */
#ifndef SPIDER_INTERNAL_H
#define SPIDER_INTERNAL_H

#include "include/spider.h"

#include "external/mongoose/mongoose.h"

extern const char* const USER_AGENT;  /**< User-Agent header for making requests to target site */
extern const char* const TARGET_HOST; /**< Hostname of target site */
extern const char* const URL_FMT;     /**< Target site URL format with placeholders for arguments */
extern const char* const OK_CODE;     /**< String format of 200 OK response code */

/**
 * @brief Optional data passed to fn_spider callback handler.
 *
 * Includes artist name, song title, done status, and error code.
 */
typedef struct
{
    char *s_artist;
    char *s_song;
    bool  done;
    int   cz_errno;
} FnSpiderData;

/**
 * @brief Event handler for Spider.
 *
 * Initiates connection with target site and handles response.
 * @see https://cesanta.com/docs/#event-handler-function
 *
 * @param[in] c       a connection that received an event 
 * @param[in] ev      an event number
 * @param[in] ev_data points to event-specific data
 * @param[in] fn_data points to user-defined data (FnSpiderData in this case)
 */
static void fn_spider (struct mg_connection *c, int ev, void *ev_data, void *fn_data);

/**
 * @brief Sends HTTP GET request to target site.
 *
 * Creates header and sends GET request to target site.
 *
 * @param[in] c        connection @see http://cesanta.com/docs/#struct-mg_connections
 * @param[in] s_artist artist name 
 * @param[in] s_song   song title 
 */
static void send_request (struct mg_connection *c, char *s_artist, char *s_song);

static void handle_response (void);
static void write_to_file (void);

/**
 * @brief Creates a new FnSpiderData struct with default values.
 *
 * Creates a new FnSpiderData struct with provided artist name and song title, and initializes
 * done status and error code to default.
 *
 * @param[in]  artist artist name 
 * @param[in]  song   song title 
 * @param[out] data   new FnSpiderData struct
 */
static FnSpiderData *new_fn_spider_data (char *artist, char *song);

/**
 * @brief Frees FnSpiderData struct.
 *
 * Frees FnSpiderData struct and internal members (if need be).
 *
 * @param[in, out] data FnSpiderData struct to be freed 
 */
static void free_fn_spider_data (FnSpiderData **data);

#endif /* SPIDER_INTERNAL_H */
