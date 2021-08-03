#include "external/mongoose/mongoose.h"

#include "czlyrics-spider.h"

static const char              *s_host = "www.songlyrics.com";
static const char              *s_artist;
static const char              *s_song;
static int                      err;

static void
fn (struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_CONNECT)
    {
        mg_printf (c,
                   "GET http://%s/%s/%s-lyrics/ HTTP/1.0\r\n"
                   "Host: %.*s\r\n"
                   "\r\n",
                   s_host, s_artist, s_song,
                   sizeof (s_host), s_host);
    }
    else if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message          *hm;
        hm = (struct mg_http_message *)  ev_data;
        char                             buffer[1024];

        // Write html to file
        snprintf (buffer, sizeof (buffer), "./cache/%s_%s.html", s_artist, s_song);
        err = mg_file_printf (buffer, "%.*s", (int) hm->body.len, hm->body.ptr) == 0;

        c->is_closing = 1;
        *(bool *) fn_data = true;
    }
    else if (ev == MG_EV_ERROR)
    {
        err = 1;
        *(bool *) fn_data = true;
    }
}

int
scrape_lyrics (const char *artist, const char *song)
{
    struct mg_mgr mgr;
    s_artist = artist;
    s_song = song;
    char *url = malloc (4096);
    snprintf (url, 4096, "http://%s/%s/%s-lyrics/", s_host, s_artist, s_song);
    bool done = false;
    mg_log_set ("3");
    mg_mgr_init (&mgr);
    mg_http_connect (&mgr, url, fn, &done);
    while (!done)
        mg_mgr_poll (&mgr, 1000);
    mg_mgr_free (&mgr);
    free (url);
    return err;
}
