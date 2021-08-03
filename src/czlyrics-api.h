#ifndef CZLYRICS_API_H
#define CZLYRICS_API_H

#include "external/mongoose/mongoose.h"

struct Endpoint
{
    char *artist;
    char *song;
} Endpoint;

void                    fn (struct mg_connection *c, int ev, void *ev_data, void *fn_data);
static struct Endpoint *extract_uri (const char *s_url);
static char            *extract_lyrics (struct Endpoint *endpoint);


#endif /* CZLYRICS_API_H */
