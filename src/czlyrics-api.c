#include "czlyrics-api.h"

#include "external/mjson/mjson.h"
#include "external/mongoose/mongoose.h"

#include "czlyrics-spider.h"

static const char *s_lyrics_endpoint = "/api/lyrics/?*/?*/";

static void
free_endpoint (struct Endpoint **endpoint)
{
    free ((*endpoint)->song);
    free ((*endpoint)->artist);
    free (*endpoint);
}

static void
free_song_data (struct SongData **song_data)
{
    free ((*song_data)->song_lyrics);
    free ((*song_data)->song_title);
    free ((*song_data)->artist_name);
    free (*song_data);
}

void
fn (struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        char                   *json = NULL;

        /* /api/lyrics/{artist}/{song}/ */
        if (mg_http_match_uri (hm, s_lyrics_endpoint))
        {
            int err;
            struct Endpoint *endpoint = malloc (sizeof (Endpoint));

            // Extract artist and song name from URI 
            if ( (err = extract_uri (hm->uri.ptr, &endpoint)) != 0 )
            {
                mjson_printf (mjson_print_dynamic_buf, &json,
                        "{%Q:{%Q:%d,%Q:%Q},%Q:{%Q:%Q,%Q:%Q,%Q:%Q}}", 
                        "error", "code", 500, "message", "Internal Server Error",
                        "data", "artist", "", "song", "", "lyrics", "");
                mg_http_reply (c, 500, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n", "%s", json);

                return;
            }

            // Scrape lyrics from target site
            if ( (err = scrape_lyrics (endpoint->artist, endpoint->song)) != 0 )
            {
                mjson_printf (mjson_print_dynamic_buf, &json, 
                        "{%Q:{%Q:%d,%Q:%Q},%Q:{%Q:%Q,%Q:%Q,%Q:%Q}}", 
                        "error", "code", 404, "message", "Song not found",
                        "data", "artist", "", "song", "", "lyrics", "");
                mg_http_reply (c, 404, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n", "%s", json);

                free_endpoint (&endpoint);
                return;
            }

            // Extract lyrics and send back to client
            struct SongData *song_data = malloc (sizeof (SongData));
            if ( (err = extract_lyrics (endpoint, &song_data) != 0))
            {
                mjson_printf (mjson_print_dynamic_buf, &json, 
                        "{%Q:{%Q:%d,%Q:%Q},%Q:{%Q:%Q,%Q:%Q,%Q:%Q}}", 
                        "error", "code", 500, "message", "Internal Server Error",
                        "data", "artist", "", "song", "", "lyrics", "");
                mg_http_reply (c, 500, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n", "%s", json);
            }
            else
            {
                mjson_printf (mjson_print_dynamic_buf, &json, 
                        "{%Q:{%Q:%d,%Q:%Q},%Q:{%Q:%Q,%Q:%Q,%Q:%Q}}", 
                        "error", "code", 200, "message", "Success",
                        "data", "artist", song_data->artist_name, "song", song_data->song_title,
                        "lyrics", song_data->song_lyrics);
                mg_http_reply (c, 200, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n", "%s", json);
            }
            free_song_data (&song_data);
            free_endpoint (&endpoint);
        }
        else
        {
            mjson_printf (mjson_print_dynamic_buf, &json, 
                    "{%Q:{%Q:%d,%Q:%Q},%Q:{%Q:%Q,%Q:%Q,%Q:%Q}}", 
                    "error", "code", 400, "message", "Bad request -- Usage: /api/lyrics/{artist}/{song}/",
                    "data", "artist", "", "song", "", "lyrics", "");
            mg_http_reply (c, 400, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n", "%s", json);
        }
    }
    (void) fn_data;
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
