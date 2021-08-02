#include "external/mongoose/mongoose.h"

#include "czlyrics-spider.h"

static const char              *s_url = "http://www.songlyrics.com/eminem/without-me-lyrics/";
static int                      err;

static void
fn (struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_CONNECT)
    {
        struct mg_str host = mg_url_host (s_url);
        mg_printf (c,
                   "GET %s HTTP/1.0\r\n"
                   "Host: %.*s\r\n"
                   "\r\n",
                   mg_url_uri (s_url), (int) host.len, host.ptr);
    }
    else if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message          *hm;
        hm = (struct mg_http_message *)  ev_data;
        char                             buffer[1024];
        static const char               *artist = "eminem";
        static const char               *song = "without-me";
        // Write html to file
        snprintf (buffer, sizeof (buffer), "./cache/%s_%s.html", artist, song);
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
scrape_lyrics (const char *url)
{
    struct mg_mgr mgr;
    bool done = false;
    mg_log_set ("3");
    mg_mgr_init (&mgr);
    mg_http_connect (&mgr, s_url, fn, &done);
    while (!done)
        mg_mgr_poll (&mgr, 1000);
    mg_mgr_free (&mgr);
    return err;
}
