#include "spider-internal.h"

#include "external/mongoose/mongoose.h"

const char* const USER_AGENT  = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
                                "Chrome/44.0.2403.157 Safari/537.36";
const char* const TARGET_HOST = "www.azlyrics.com";
const char* const URL_FMT     = "https://www.azlyrics.com/lyrics/%s/%s.html";
const char* const OK_CODE     = "200";

/* PUBLIC METHODS */

int
scrape_lyrics (const char *artist, const char *song)
{
    struct mg_mgr        mgr;
    char                 url[1024];
    struct FnSpiderData *data;
    int                  cz_errno;

    data = new_fn_spider_data ((char *) artist, (char *) song);

    snprintf (url, 1024, URL_FMT, artist, song);

    mg_log_set ("3");
    mg_mgr_init (&mgr);
    mg_http_connect (&mgr, url, fn_spider, data);
    while (!(data->done))
        mg_mgr_poll (&mgr, 1000);
    mg_mgr_free (&mgr);

    cz_errno = data->cz_errno;
    free_fn_spider_data (&data);

    return cz_errno;
}

/* INTERNAL METHODS */

static void
fn_spider (struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_CONNECT)
    {
        struct mg_tls_opts opts = {.srvname = TARGET_HOST};
        mg_tls_init(c, &opts);
        send_request (c, ((struct FnSpiderData *) fn_data)->s_artist, ((struct FnSpiderData *) fn_data)->s_song);
    }
    else if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm;
        char                   *response_code;

        hm = (struct mg_http_message *) ev_data;

        strncpy (response_code, hm->uri.ptr, (size_t) hm->uri.len);
        if (strcmp (OK_CODE, response_code) == 0)
        {
            // Write html to file
            char buffer[1024];
            snprintf (buffer,
                      sizeof (buffer),
                      "./cache/%s_%s.html",
                      ((struct FnSpiderData *) fn_data)->s_artist,
                      ((struct FnSpiderData *) fn_data)->s_song);
            ((struct FnSpiderData *) fn_data)->cz_errno = mg_file_printf (buffer,
                                                                          "%.*s",
                                                                          (int) hm->body.len,
                                                                          hm->body.ptr) == 0;
        }
        else
            ((struct FnSpiderData *) fn_data)->cz_errno = 1;

        c->is_closing = 1;
        ((struct FnSpiderData *) fn_data)->done = true;
    }
    else if (ev == MG_EV_ERROR)
    {
        ((struct FnSpiderData *) fn_data)->cz_errno = 1;
        ((struct FnSpiderData *) fn_data)->done = true;
    }
}

static void
send_request (struct mg_connection *c, char *s_artist, char *s_song)
{
    mg_printf (c,
               "GET https://%s/lyrics/%s/%s.html HTTP/1.0\r\n"
               "Host: %s\r\n"
               "User-Agent: %s\r\n"
               "\r\n",
               TARGET_HOST, s_artist, s_song, TARGET_HOST, USER_AGENT);
}

static void
handle_response (void)
{
}

static void
write_to_file (void)
{
}

static struct FnSpiderData *
new_fn_spider_data (char *artist, char *song)
{
    struct FnSpiderData *data;

    data = malloc (sizeof (struct FnSpiderData));
    data->s_artist = strdup (artist);
    data->s_song = strdup (song);
    data->done = false;
    data->cz_errno = 0;

    return data;
}

static void
free_fn_spider_data (struct FnSpiderData **data)
{
    free ((*data)->s_artist);
    free ((*data)->s_song);
    free (*data);
}
