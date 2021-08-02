#include "czlyrics-api.h"

#include "external/mjson/mjson.h"
#include "external/mongoose/mongoose.h"

#include "czlyrics-spider.h"

static const char *s_lyrics_endpoint = "/api/lyrics/*/*";
static int         err;

char *extract_lyrics (const char *f_path);

void
fn (struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        char                   *json = NULL;

        if (mg_http_match_uri (hm, s_lyrics_endpoint))
        {
            if ((err = scrape_lyrics (s_lyrics_endpoint)) == 0)
            {
                char *s_lyrics = extract_lyrics ("./cache/eminem_without-me.html");
                mjson_printf (mjson_print_dynamic_buf, &json, "{%Q:{%Q:%Q}}", "data", "lyrics", s_lyrics);
                mg_http_reply (c, 200, "Content-Type: application/json\r\n", "%s", json);
                free (s_lyrics);
            }
            else 
            {
                mjson_printf (mjson_print_dynamic_buf, &json, 
                        "{%Q:{%Q:%d,%Q:%Q}}", "error", "code", 404, "message", "Lyrics not found");
                mg_http_reply (c, 404, "", "%s", json);
            }
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

char *
extract_lyrics (const char *f_path)
{
    FILE          *fp;
    char          *f_data;
    long           f_size;
    char          *s_lyrics;
    const char    *match = "songLyricsV14 iComment-text";

    // Open html file and allocate buffer for contents
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

    // Scan lyrics and replace any br tags
    char *cur_start = cur;
    int pos = 0;
    while (cur < cur_end)
    {
        if (*cur == '<')              /* start of <br /> tag */
        {
            cur += 6;                 /* tag is 6 chars long */
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
