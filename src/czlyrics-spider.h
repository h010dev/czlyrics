#ifndef CZLYRICS_SPIDER_H
#define CZLYRICS_SPIDER_H

#include "external/mongoose/mongoose.h"

extern const char* const USER_AGENT;  /* User-Agent header for making requests to target site */
extern const char* const TARGET_HOST; /* Hostname of target site */
extern const char* const URL_FMT;     /* Target site URL format with placeholders for arguments */
extern const char* const OK_CODE;     /* String format of 200 OK response code */

struct FnSpiderData
{
    char *s_artist;
    char *s_song;
    bool  done;
    int   cz_errno;
} FnSpiderData; /* Contains arguments passed to fn_spider's fn_data param */

int scrape_lyrics (const char *artist, const char *song);
void fn_spider (struct mg_connection *c, int ev, void *ev_data, void *fn_data);
void send_request (struct mg_connection *c, char *s_artist, char *s_song);
struct FnSpiderData *new_fn_spider_data (char *artist, char *song);
void free_fn_spider_data (struct FnSpiderData **data);
void cz_handle_response (void);
void cz_write_to_file (void);

#endif /* CZLYRICS_SPIDER_H */
