#include "external/mjson/mjson.h"
#include "external/mongoose/mongoose.h"

#include "czlyrics-api.h"
#include "czlyrics-spider.h"
#include "czlyrics-parser.h"

const char* const CZ_API_ENDPOINT            = "/api/lyrics/?*/?*/";
const char* const CZ_RESPONSE_HEADER_OPTIONS = "Content-Type: application/json\r\n"
                                               "Access-Control-Allow-Origin: *\r\n";
const char* const CZ_HTTP_MSG_OK             = "Success";
const char* const CZ_HTTP_MSG_BAD_REQUEST    = "Bad request -- Usage: /api/lyrics/{artist}/{song}/";
const char* const CZ_HTTP_MSG_NOT_FOUND      = "Song not found";
const char* const CZ_HTTP_MSG_SERVER_ERROR   = "Internal Server Error";

void
fn_api (struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_HTTP_MSG)
        cz_handle_request (c, ev_data);

    (void) fn_data;
}

void
cz_handle_request (struct mg_connection *c, void *ev_data)
{
    struct mg_http_message *hm;
    struct SongData        *song_data;

    hm        = (struct mg_http_message *) ev_data;
    song_data = NULL;

    if (mg_http_match_uri (hm, CZ_API_ENDPOINT))
        cz_get_lyrics (c, hm);
    else
        send_response (c, BAD_REQUEST, song_data);
}

void
cz_get_lyrics (struct mg_connection *c, struct mg_http_message *hm)
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

void
create_response_body (int http_err_code, const char *message, struct SongData *song_data, char **response_body)
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
                  "error", "code", http_err_code, "message", message,
                  "data", "artist", artist, "song", song, "lyrics", lyrics);

    *response_body = strdup (json);
    free (json);
}

void
send_response (struct mg_connection *c, int http_err_code, struct SongData *song_data)
{
    char *response_body;

    response_body = "";

    switch (http_err_code)
    {
        case OK:
            create_response_body (http_err_code, CZ_HTTP_MSG_OK, song_data, &response_body);
            break;
        case BAD_REQUEST:
            create_response_body (http_err_code, CZ_HTTP_MSG_BAD_REQUEST, song_data, &response_body);
            break;
        case NOT_FOUND:
            create_response_body (http_err_code, CZ_HTTP_MSG_NOT_FOUND, song_data, &response_body);
            break;
        case SERVER_ERROR:
            create_response_body (http_err_code, CZ_HTTP_MSG_SERVER_ERROR, song_data, &response_body);
            break;
        default:
            break;
    }

    mg_http_reply (c, http_err_code, CZ_RESPONSE_HEADER_OPTIONS, "%s", response_body);
}

