#include "external/mjson/mjson.h"
#include "external/mongoose/mongoose.h"

#include "czlyrics-api.h"
#include "czlyrics-spider.h"

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
        send_response (c, 400, song_data);
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
    if ( (cz_errno = extract_uri (hm->uri.ptr, &endpoint)) != 0 )
    {
        send_response (c, 500, song_data);
        return;
    }

    // Scrape lyrics from target site
    if ( (cz_errno = scrape_lyrics (endpoint->artist, endpoint->song)) != 0 )
    {
        send_response (c, 404, song_data);
        free_endpoint (&endpoint);
        return;
    }

    // Extract lyrics and send back to client

    song_data = malloc (sizeof (SongData));
    if ( (cz_errno = extract_lyrics (endpoint, &song_data) != 0))
        send_response (c, 500, song_data);
    else
        send_response (c, 200, song_data);

    free_song_data (&song_data);
    free_endpoint (&endpoint);
}

int
extract_uri (const char *s_url, struct Endpoint **endpoint)
{
    char       *artist = NULL;
    char       *song   = NULL;
    const char *match  = "/api/lyrics/";

    // allocate space for return struct
    if ( (artist = calloc (255, sizeof (char))) == NULL)
    {
        printf ("Failed to allocate space for artist.\n");
        return 1;
    }

    if ( (song = calloc (255, sizeof (char))) == NULL)
    {
        printf ("Failed to allocate space for song.\n");
        return 1;
    }

    // seek forward past api url (api/lyrics)
    char *cur;
    cur = strstr (s_url, match);
    cur += strlen (match);

    // extract artist name
    int pos = 0;
    while (*cur != '/')
    {
        // skip over all non-alphanumeric chars
        if (isalnum (*cur))
        {
            artist[pos] = tolower (*cur);  /* lowercase all chars for consistent file naming and access */
            pos++;
        }
        // skip over whitespace (%20)
        else if (*cur == '%')
            cur += 2;
        cur++;
    }
    cur++;
    printf ("ARTIST NAME = %s\n", artist);

    // extract song name
    pos = 0;
    while (*cur != '/')
    {
        // skip over all non-alphanumeric chars
        if (isalnum (*cur))
        {
            song[pos] = tolower (*cur);  /* lowercase all chars for consistent file naming and access */
            pos++;
        }
        // skip over whitespace (%20)
        else if (*cur == '%')
            cur += 2;
        cur++;
    }

    (*endpoint)->artist = artist;
    (*endpoint)->song = song;

    return 0;
}

int
extract_lyrics (struct Endpoint *endpoint, struct SongData **song_data)
{
    // Open html file 
    FILE *fp;
    char  f_path[1024];

    snprintf (f_path, sizeof (f_path), "./cache/%s_%s.html", endpoint->artist, endpoint->song);
    if ( (fp = fopen (f_path, "rb")) == NULL)
    {
        printf ("File couldn't be opened.\n");
        return 1;
    }

    // Allocate buffer for file contents
    char *f_data;
    long  f_size;

    fseek (fp, 0L, SEEK_END);
    f_size = ftell (fp);
    rewind (fp);

    if ( (f_data = calloc (1, f_size + 1)) == NULL)
    {
        fclose (fp);
        free (f_data);
        printf ("Failed to allocate space for file.\n");
        return 1;
    }

    // Read data from file into file buffer
    if (fread (f_data, f_size, 1, fp) != 1)
    {
        fclose (fp);
        free (f_data);
        printf ("File could not be read.\n");
        return 1;
    }
    fclose (fp);

    // Extract content from file
    char *cur, *cur_end, *end_match;
    int pos;

    // Seek to artist name 
    char       *s_artist_name;
    const char *artist_match = "ArtistName = ";

    cur = strstr (f_data, artist_match);    /* artist name starts here */
    cur += strlen (artist_match);           /* seek to start of artist name */
    cur++;                                  /* skip past opening quotation mark */
    end_match = "\";";                      /* end of artist name */
    cur_end = strstr (cur, end_match);      /* seek to end of artist name */

    // Allocate space for artist name
    if ( (s_artist_name = calloc (1, (char *) cur_end - (char *) cur + 1 )) == NULL )
    {
        free (f_data);
        printf ("Failed to allocate space for artist name.\n");
        return 1;
    }

    // Extract artist name
    pos = 0;
    while (cur < cur_end)
    {
        s_artist_name[pos] = cur[0];
        cur++;
        pos++;
    }

    // Seek to song title 
    char       *s_song_title;
    const char *song_match = "SongName = ";

    cur = strstr (f_data, song_match);      /* song title starts here */
    cur += strlen (song_match);             /* seek to start of song title */
    cur++;                                  /* skip past opening quotation mark */
    end_match = "\";";                      /* end of song title */
    cur_end = strstr (cur, end_match);      /* seek to end of song title */

    // Allocate space for song title 
    if ( (s_song_title = calloc (1, (char *) cur_end - (char *) cur + 1 )) == NULL )
    {
        free (f_data);
        printf ("Failed to allocate space for song title.\n");
        return 1;
    }

    // Extract song title 
    pos = 0;
    while (cur < cur_end)
    {
        s_song_title[pos] = cur[0];
        cur++;
        pos++;
    }

    // Seek to song lyrics
    char       *s_song_lyrics;
    const char *lyrics_match = "Sorry about that. -->";

    cur = strstr (f_data, lyrics_match);      /* lyrics start here */
    cur += strlen (lyrics_match);             /* seek to start of lyrics */
    end_match = "</div>";                     /* a closing div tag indicates end of lyrics */
    cur_end = strstr (cur, end_match);        /* seek to end of song lyrics */ 

    // Allocate space for lyrics
    if ( (s_song_lyrics = calloc (1, (char *) cur_end - (char *) cur + 1)) == NULL)
    {
        free (f_data);
        printf ("Failed to allocate space for lyrics.\n");
        return 1;
    }

    // Scan lyrics and replace any tags
    pos = 0;
    while (cur < cur_end)
    {
        if (*cur == '<')              /* start of a tag */
        {
            while (*cur != '>')       /* seek to end of tag */
                cur++;
            cur++;                    /* seek past closing tag */
            continue;
        }
        else if (*cur == '&')         /* start of an html entity/symbol */
        {
            while (*cur != ';')       /* seek to end of symbol */
                cur++;
            cur++;                    /* seek past semicolon */
            continue;
        }
        else
        {
            s_song_lyrics[pos] = cur[0];
            cur++;
        }
        pos++;
    }

    (*song_data)->artist_name = s_artist_name;
    (*song_data)->song_title = s_song_title;
    (*song_data)->song_lyrics = s_song_lyrics;

    return 0;
}

void
create_response_body (int http_err_code, const char *message, struct SongData *song_data, char **response_body)
{
    char *artist = "";
    char *song   = "";
    char *lyrics = "";
    char *json   = NULL;

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

void
free_endpoint (struct Endpoint **endpoint)
{
    free ((*endpoint)->song);
    free ((*endpoint)->artist);
    free (*endpoint);
}

void
free_song_data (struct SongData **song_data)
{
    free ((*song_data)->song_lyrics);
    free ((*song_data)->song_title);
    free ((*song_data)->artist_name);
    free (*song_data);
}

