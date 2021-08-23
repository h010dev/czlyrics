/**
 * @file   api-internal.h 
 * @author Mohamed Al-Hussein
 * @date   08/21/2021
 * @brief  Internal header file for API.
 */
#ifndef API_INTERNAL_H
#define API_INTERNAL_H

#include "include/api.h"

#include "external/mongoose/mongoose.h"

#include "include/parser.h"

extern const char* const API_ENDPOINT;            /**< API glob route pattern */
extern const char* const RESPONSE_HEADER_OPTIONS; /**< Header options for HTTP response sent back to client */
extern const char* const HTTP_MSG_OK;             /**< String message for HTTP 200 OK */
extern const char* const HTTP_MSG_BAD_REQUEST;    /**< String message for HTTP 400 Bad Request */
extern const char* const HTTP_MSG_NOT_FOUND;      /**< String message for HTTP 404 Not Found */
extern const char* const HTTP_MSG_SERVER_ERROR;   /**< String message for HTTP 500 Internal Server Error */
extern const char* const HTTP_BODY_TEMPLATE;      /**< HTTP JSON body template string */
extern const char* const http_msg[];              /**< Mapping between HTTP status codes and HTTP status messages */

/**
 * @brief HTTP status codes.
 *
 * Only includes status codes used by API.
 */
typedef enum
{
    OK           = 200,
    BAD_REQUEST  = 400,
    NOT_FOUND    = 404,
    SERVER_ERROR = 500
} http_code;

/**
 * @brief Handles all HTTP requests made to API.
 *
 * Checks if http message contained in @p hm matches API endpoint.
 *   If there is a match, an attempt is made to fetch the lyrics for the given artist and song title.
 *   Otherwise, an HTTP 400 response is sent back to client.
 *
 * @param[in] c  connection @see http://cesanta.com/docs/#struct-mg_connections
 * @param[in] hm http message @see http://cesanta.com/docs/#struct-mg_http_messages
 */
static void handle_request (struct mg_connection *c, struct mg_http_message *hm);

/**
 * @brief HTTP GET route for API that returns the lyrics for a given artist and song title.
 *
 * This is achieved in a 3-step process:
 *   1. Attempts to parse HTTP GET message to determine artist name and song title. 
 *   2. Attempts to scrape lyrics off target site using given artist name and song title (delegated to spider).
 *   3. Attempts to parse the lyrics and other data (artist, song title) from the downloaded HTML file, and sends
 *      data as a HTTP response to client.
 *
 * If anything goes wrong internally, sends HTTP 500 response back to client.
 * If the song was not found, sends HTTP 404 response back to client.
 *
 * @param[in] c  connection @see http://cesanta.com/docs/#struct-mg_connections
 * @param[in] hm http message @see http://cesanta.com/docs/#struct-mg_http_messages
 */
static void get_lyrics (struct mg_connection *c, struct mg_http_message *hm);

/**
 * @brief Sends HTTP response back to client.
 *
 * Inspects @p status and sends appropriate response to client. 
 * Optionally includes @p song_data if @p status is 200.
 *
 * @param[in] c             connection @see http://cesanta.com/docs/#struct-mg_connections
 * @param[in] http_err_code http status code
 * @param[in] song_data     song data to be sent (can be NULL)
 */
static void send_response (struct mg_connection *c, http_code status, SongData *song_data);

/**
 * @brief Creates a JSON formatted response body containing the HTTP status code and message, and song data (if any).
 *
 * All JSON response bodies contain an overview of the http status code and accompanying message,
 * the artist name, song title, and song lyrics.
 * @see https://github.com/h010dev/czlyrics/blob/main/README.md for information on the JSON response format. 
 *
 * Fields are empty if nothing was found (e.g. song not found).
 *
 * @param[in]      http_err_code http status code
 * @param[in]      message       error message
 * @param[in]      song_data     song information
 * @param[in, out] response_body newly created response body
 */
static void create_response_body (http_code status, const char *message, SongData *song_data, char **response_body);

#endif /* API_INTERNAL_H */
