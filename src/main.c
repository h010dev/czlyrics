#include <stdlib.h>

#include "external/mongoose/mongoose.h"

#include "czlyrics-api.h"

#define CZ_POLL_TIME 1000

static const char *s_log_level = "2";
static const char *s_http_addr = "http://0.0.0.0:8080";

int
main (int argc, char *argv[])
{
    struct mg_mgr mgr;

    mg_log_set (s_log_level);
    mg_mgr_init (&mgr);
    mg_http_listen (&mgr, s_http_addr, fn_api, NULL);
    for (;;)
        mg_mgr_poll (&mgr, CZ_POLL_TIME);
    mg_mgr_free (&mgr);

    return EXIT_SUCCESS;
}
