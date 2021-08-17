#ifndef CZLYRICS_API_H
#define CZLYRICS_API_H

#include "external/mongoose/mongoose.h"

extern const char* const CZ_API_ENDPOINT; /* API glob route pattern `/api/lyrics/{artist}/{song}/`*/
extern const char* const CZ_RESPONSE_HEADER_OPTIONS; /* Header options for HTTP response sent back to client */
extern const char* const CZ_HTTP_MSG_OK; /* String message for HTTP 200 OK */
extern const char* const CZ_HTTP_MSG_BAD_REQUEST; /* String message for HTTP 400 Bad Request */
extern const char* const CZ_HTTP_MSG_NOT_FOUND; /* String message for HTTP 404 Not Found */
extern const char* const CZ_HTTP_MSG_SERVER_ERROR; /* String message for HTTP 500 Internal Server Error */

struct Endpoint
{
    char *artist;
    char *song;
} Endpoint; /* endpoint split by artist name and song title */

struct SongData
{
    char *artist_name;
    char *song_title;
    char *song_lyrics;
} SongData; /* extracted song data */

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
 * Parses the request URI and fills out an Endpoint struct containing the artist name
 * and song title.
 *
 * Returns 0 on success and 1 on failure.
 */
int extract_uri (const char *s_url, struct Endpoint **endpoint);

/*
 * Parses lyrics for given Endpoint and fills out a SongData struct containing the 
 * artist name, song tile, and song lyrics.
 *
 * Returns 0 on success and 1 on failure.
 */
int extract_lyrics (struct Endpoint *endpoint, struct SongData **song_data);

/*
 * Creates JSON formatted response body.
 */
void create_response_body (int http_err_code, const char *message, struct SongData *song_data, char **response_body);

/*
 * Sends HTTP response to client.
 */
void send_response (struct mg_connection *c, int http_err_code, struct SongData *song_data);

/*
 * Free memory for Endpoint.
 */
void free_endpoint (struct Endpoint **endpoint);

/*
 * Free memory for SongData.
 */
void free_song_data (struct SongData **song_data);

#endif /* CZLYRICS_API_H */
