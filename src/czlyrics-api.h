#ifndef CZLYRICS_API_H
#define CZLYRICS_API_H

#include "external/mongoose/mongoose.h"

struct Endpoint
{
    char *artist;
    char *song;
} Endpoint;

struct SongData
{
    char *artist_name;
    char *song_title;
    char *song_lyrics;
} SongData;

void                    fn (struct mg_connection *c, int ev, void *ev_data, void *fn_data);
static struct Endpoint *extract_uri (const char *s_url);
int                     extract_lyrics (struct Endpoint *endpoint, struct SongData **song_data);


#endif /* CZLYRICS_API_H */
