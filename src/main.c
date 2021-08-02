#include <stdlib.h>

#include "external/mongoose/mongoose.h"

#include "czlyrics-api.h"

static const char *s_http_addr = "http://0.0.0.0:8080";

int
main (int argc, char *argv[])
{
    struct mg_mgr mgr;
    mg_log_set ("2");
    mg_mgr_init (&mgr);
    mg_http_listen (&mgr, s_http_addr, fn, NULL);
    for (;;)
        mg_mgr_poll (&mgr, 1000);
    mg_mgr_free (&mgr);
    return EXIT_SUCCESS;
}
