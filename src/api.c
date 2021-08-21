#include "api-internal.h"

#include "external/mjson/mjson.h"
#include "external/mongoose/mongoose.h"

#include "include/spider.h"
#include "include/parser.h"

const char* const API_ENDPOINT            = "/api/lyrics/?*/?*/";
const char* const RESPONSE_HEADER_OPTIONS = "Content-Type: application/json\r\n"
                                            "Access-Control-Allow-Origin: *\r\n";
const char* const HTTP_MSG_OK             = "Success";
const char* const HTTP_MSG_BAD_REQUEST    = "Bad request -- Usage: /api/lyrics/{artist}/{song}/";
const char* const HTTP_MSG_NOT_FOUND      = "Song not found";
const char* const HTTP_MSG_SERVER_ERROR   = "Internal Server Error";

/* PUBLIC METHODS */

void
fn_api (struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_HTTP_MSG)
        handle_request (c, (struct mg_http_message *) ev_data);

    (void) fn_data;
}

/* INTERNAL METHODS */

static void
handle_request (struct mg_connection *c, struct mg_http_message *hm)
{
    if (mg_http_match_uri (hm, API_ENDPOINT))
        get_lyrics (c, hm);
    else
    {
        struct SongData *song_data;

        song_data = NULL;
        send_response (c, BAD_REQUEST, song_data);
    }
}

static void
get_lyrics (struct mg_connection *c, struct mg_http_message *hm)
{
    int              cz_errno;
    struct Endpoint *endpoint;
    struct SongData *song_data;

    endpoint  = malloc (sizeof (Endpoint));
    song_data = NULL;

    // Extract artist and song name from URI 
    if ( (cz_errno = parse_url (hm->uri.ptr, &endpoint)) != 0 )
    {
        send_response (c, SERVER_ERROR, song_data);
        return;
    }

    // Scrape lyrics from target site
    if ( (cz_errno = scrape_lyrics (endpoint->artist, endpoint->song)) != 0 )
    {
        send_response (c, NOT_FOUND, song_data);
        free_endpoint (&endpoint);
        return;
    }

    // Extract lyrics and send back to client
    song_data = malloc (sizeof (SongData));
    if ( (cz_errno = parse_song_data (endpoint, &song_data) != 0) )
        send_response (c, SERVER_ERROR, song_data);
    else
        send_response (c, OK, song_data);

    free_song_data (&song_data);
    free_endpoint (&endpoint);
}

static void
create_response_body (http_code status, const char *message, struct SongData *song_data, char **response_body)
{
    char *artist, *song, *lyrics, *json;

    artist = "";
    song   = "";
    lyrics = "";
    json   = NULL;

    if (song_data != NULL)
    {
        artist = strdup (song_data->artist_name);
        song = strdup (song_data->song_title);
        lyrics = strdup (song_data->song_lyrics);
    }

    mjson_printf (mjson_print_dynamic_buf, &json,
                  "{%Q:{%Q:%d,%Q:%Q},%Q:{%Q:%Q,%Q:%Q,%Q:%Q}}", 
                  "error", "code", status, "message", message,
                  "data", "artist", artist, "song", song, "lyrics", lyrics);

    *response_body = strdup (json);
    free (json);
}

static void
send_response (struct mg_connection *c, http_code status, struct SongData *song_data)
{
    char *response_body;

    response_body = "";

    switch (status)
    {
        case OK:
            create_response_body (status, HTTP_MSG_OK, song_data, &response_body);
            break;
        case BAD_REQUEST:
            create_response_body (status, HTTP_MSG_BAD_REQUEST, song_data, &response_body);
            break;
        case NOT_FOUND:
            create_response_body (status, HTTP_MSG_NOT_FOUND, song_data, &response_body);
            break;
        case SERVER_ERROR:
            create_response_body (status, HTTP_MSG_SERVER_ERROR, song_data, &response_body);
            break;
        default:
            break;
    }

    mg_http_reply (c, status, RESPONSE_HEADER_OPTIONS, "%s", response_body);
}

