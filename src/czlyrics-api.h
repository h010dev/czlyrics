#ifndef CZLYRICS_API_H
#define CZLYRICS_API_H

#include "external/mongoose/mongoose.h"

#include "czlyrics-parser.h"

extern const char* const CZ_API_ENDPOINT; /* API glob route pattern `/api/lyrics/{artist}/{song}/`*/
extern const char* const CZ_RESPONSE_HEADER_OPTIONS; /* Header options for HTTP response sent back to client */
extern const char* const CZ_HTTP_MSG_OK; /* String message for HTTP 200 OK */
extern const char* const CZ_HTTP_MSG_BAD_REQUEST; /* String message for HTTP 400 Bad Request */
extern const char* const CZ_HTTP_MSG_NOT_FOUND; /* String message for HTTP 404 Not Found */
extern const char* const CZ_HTTP_MSG_SERVER_ERROR; /* String message for HTTP 500 Internal Server Error */

enum cz_http_code 
{
    OK           = 200,
    BAD_REQUEST  = 400,
    NOT_FOUND    = 404,
    SERVER_ERROR = 500
}; /* Supported HTTP status codes */

/*
 * Listens for incoming connections to API.
 */
void fn_api (struct mg_connection *c, int ev, void *ev_data, void *fn_data);

/*
 * Handles HTTP requests.
 */
void cz_handle_request (struct mg_connection *c, void *ev_data);

/*
 * HTTP GET route for lyrics.
 */
void cz_get_lyrics (struct mg_connection *c, struct mg_http_message *hm);

/*
 * Creates JSON formatted response body.
 */
void create_response_body (int http_err_code, const char *message, struct SongData *song_data, char **response_body);

/*
 * Sends HTTP response to client.
 */
void send_response (struct mg_connection *c, int http_err_code, struct SongData *song_data);

#endif /* CZLYRICS_API_H */
