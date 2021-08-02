#include "czlyrics-api.h"

#include "external/mongoose/mongoose.h"

#include "czlyrics-spider.h"

static const char *s_lyrics_endpoint = "/api/lyrics/*/*";
static int         err;

void
fn (struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        if (mg_http_match_uri (hm, s_lyrics_endpoint))
        {
            if ((err = scrape_lyrics (s_lyrics_endpoint)) == 0)
            {
                mg_http_reply (c, 200, "", "200: OK");
            }
            else 
            {
                mg_http_reply (c, 404, "", "404: Not found");
            }
        }
        else
        {
            mg_http_reply (c, 400, "", "400: Bad request\r\n\r\nUsage: /api/lyrics/artist/song");
        }
    }
    (void) fn_data;
}
