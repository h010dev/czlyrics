#include "external/mongoose/mongoose.h"

#include "czlyrics-spider.h"

static const char              *s_user_agent = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36";
static const char              *s_host = "www.azlyrics.com";
static const char              *s_artist;
static const char              *s_song;
static int                      err;

static void
fn (struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_CONNECT)
    {
        struct mg_tls_opts opts = {.srvname = s_host};
        mg_tls_init(c, &opts);
        mg_printf (c,
                   "GET https://%s/lyrics/%s/%s.html HTTP/1.0\r\n"
                   "Host: %s\r\n"
                   "User-Agent: %s\r\n"
                   "\r\n",
                   s_host, s_artist, s_song,
                   s_host,
                   s_user_agent);
    }
    else if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message          *hm;
        hm = (struct mg_http_message *)  ev_data;

        char *response_ok = "200";
        char *response_code;
        strncpy (response_code, hm->uri.ptr, (size_t) hm->uri.len);
        if (strcmp (response_ok, response_code) == 0)
        {
            // Write html to file
            char  buffer[1024];
            snprintf (buffer, sizeof (buffer), "./cache/%s_%s.html", s_artist, s_song);
            err = mg_file_printf (buffer, "%.*s", (int) hm->body.len, hm->body.ptr) == 0;
        }
        else
            err = 1;

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
    char url[1024];
    snprintf (url, 1024, "https://%s/lyrics/%s/%s.html", s_host, artist, song);
    bool done = false;
    mg_log_set ("3");
    mg_mgr_init (&mgr);
    mg_http_connect (&mgr, url, fn, &done);
    while (!done)
        mg_mgr_poll (&mgr, 1000);
    mg_mgr_free (&mgr);
    return err;
}
