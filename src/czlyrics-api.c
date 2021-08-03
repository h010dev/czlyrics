#include "czlyrics-api.h"

#include "external/mjson/mjson.h"
#include "external/mongoose/mongoose.h"

#include "czlyrics-spider.h"

static const char *s_lyrics_endpoint = "/api/lyrics/*/*";

void
fn (struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        char                   *json = NULL;

        if (mg_http_match_uri (hm, s_lyrics_endpoint))
        {
            int err;
            struct Endpoint *endpoint = NULL;
            endpoint = extract_uri (hm->uri.ptr);
            if ((err = scrape_lyrics (endpoint->artist, endpoint->song)) == 0)
            {
                char *s_uri = malloc (2048);
                char *s_lyrics = extract_lyrics (endpoint);
                mjson_printf (mjson_print_dynamic_buf, &json, 
                        "{%Q:{%Q:%Q}}", "data", "lyrics", s_lyrics);
                mg_http_reply (c, 200, "Content-Type: application/json\r\n", "%s", json);
                free (s_uri);
                free (s_lyrics);
            }
            else 
            {
                mjson_printf (mjson_print_dynamic_buf, &json, 
                        "{%Q:{%Q:%d,%Q:%Q}}", "error", "code", 404, "message", "Lyrics not found");
                mg_http_reply (c, 404, "", "%s", json);
            }
            free (endpoint);
        }
        else
        {
            mjson_printf (mjson_print_dynamic_buf, &json, 
                    "{%Q:{%Q:%d,%Q:%Q}}", "error", "code", 400, "message", "Bad request");
            mg_http_reply (c, 400, "", "%s", json);
        }
        free (json);
    }
    (void) fn_data;
}

static struct Endpoint *
extract_uri (const char *s_url)
{
    char            *artist;
    char            *song;
    const char      *match = "/api/lyrics/";
    struct Endpoint *endpoint = NULL;

    // allocate space for return struct
    endpoint = malloc (sizeof (struct Endpoint));
    if ( (artist = malloc (1024)) == NULL)
    {
        printf ("Malloc failed.\n");
        return endpoint;
    }

    if ( (song = malloc (1024)) == NULL)
    {
        printf ("Malloc failed.\n");
        return endpoint;
    }

    // seek forward past api url (api/lyrics)
    char *cur;
    cur = strstr (s_url, match);
    cur += strlen (match);

    // extract artist name
    int pos = 0;
    while (*cur != '/')
    {
        // skip over spaces (%20)
        if (*cur == '%')
        {
            // don't duplicate - back to back
            if (pos > 0 && artist[pos - 1] != '-')
            {
                artist[pos] = '-';
                pos++;
            }
            cur += 3;
            continue;
        }
        // handle all other non-alphanumeric chars
        else if (!isalnum (*cur))
        {
            // don't duplicate - back to back
            if (pos > 0 && artist[pos - 1] != '-')
            {
                artist[pos] = '-';
                pos++;
            }
            cur++;
            continue;
        }
        else 
        {
            artist[pos] = tolower(*cur);          /* lowercase all text for consistent file access */
            cur++;
            pos++;
        }
    }
    // if last character is non-alphanumeric, remove it (otherwise file name invalid)
    if (!isalnum(artist[pos - 1]))
        artist[pos - 1] = '\0';

    // extract song name
    pos = 0;
    cur++;
    while (*cur != ' ')
    {
        // skip over spaces (%20)
        if (*cur == '%')
        {
            // don't duplicate - back to back
            if (pos > 0 && song[pos - 1] != '-')
            {
                song[pos] = '-';
                pos++;
            }
            cur += 3;
            continue;
        }
        // handle all other non-alphanumeric chars
        else if (!isalnum (*cur))
        {
            // don't duplicate - back to back
            if (pos > 0 && song[pos - 1] != '-')
            {
                song[pos] = '-';
                pos++;
            }
            cur++;
            continue;
        }
        else 
        {
            song[pos] = tolower(*cur);          /* lowercase all text for consistent file access */
            cur++;
            pos++;
        }
    }
    // if last character is non-alphanumeric, remove it (otherwise file name invalid)
    if (!isalnum(song[pos - 1]))
        song[pos - 1] = '\0';

    endpoint->artist = artist;
    endpoint->song = song;
    return endpoint;
}

static char *
extract_lyrics (struct Endpoint *endpoint)
{
    FILE          *fp;
    char           f_path[1024];
    char          *f_data;
    long           f_size;
    char          *s_lyrics;
    const char    *match = "songLyricsV14 iComment-text";

    // Open html file and allocate buffer for contents
    snprintf (f_path, sizeof (f_path), "./cache/%s_%s.html", endpoint->artist, endpoint->song);
    if ( (fp = fopen (f_path, "rb")) == NULL)
    {
        printf ("File couldn't be opened.\n");
        return s_lyrics;
    }

    fseek (fp, 0L, SEEK_END);
    f_size = ftell (fp);
    rewind (fp);

    if ( (f_data = calloc (1, f_size + 1)) == NULL)
    {
        fclose (fp);
        printf ("Calloc failed.\n");
        return s_lyrics;
    }

    if (fread (f_data, f_size, 1, fp) != 1)
    {
        fclose (fp);
        free (f_data);
        printf ("File could not be read.\n");
        return s_lyrics;
    }
    fclose (fp);
    printf ("File read successful.\n");

    // Seek to match position (where lyrics will start)
    char *cur, *cur_end;
    cur = strstr (f_data, match);      /* lyrics start here */
    cur += strlen (match);             /* seek to start of lyrics */
    cur += 2;                          /* seek past quotation and closing angle bracket */
    match = "</div>";                  /* a closing div tag indicates end of lyrics */

    // Allocate space for lyrics
    cur_end = strstr (cur, match);
    if ( (s_lyrics = calloc (1, (char *) cur_end - (char *) cur + 1)) == NULL)
    {
        free (f_data);
        printf ("Calloc failed.\n");
        return s_lyrics;
    }

    // Scan lyrics and replace any tags
    char *cur_start = cur;
    int   pos = 0;
    while (cur < cur_end)
    {
        if (*cur == '<')              /* start of a tag */
        {
            while (*cur != '>')       /* seek to end of tag */
                cur++;
            cur++;                    /* seek past closing tag */
            continue;
        }
        else
        {
            s_lyrics[pos] = cur[0];
            cur++;
        }
        pos++;
    }

    free (f_data);
    return s_lyrics;
}
