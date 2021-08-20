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

/*
 * Given an artist and song, scrape lyrics off target site with matching values.
 */
int scrape_lyrics (const char *artist, const char *song);

/*
 * Callback handler responsible for initiating connection with target server
 * and requesting data.
 */
void fn_spider (struct mg_connection *c, int ev, void *ev_data, void *fn_data);

/*
 * Send HTTP GET request to target site.
 */
void send_request (struct mg_connection *c, char *s_artist, char *s_song);

void cz_handle_response (void);
void cz_write_to_file (void);

/*
 * Create a new FnSpiderData struct with default values
 */
struct FnSpiderData *new_fn_spider_data (char *artist, char *song);

/*
 * Free FnSpiderData struct and relevant members.
 */
void free_fn_spider_data (struct FnSpiderData **data);

#endif /* CZLYRICS_SPIDER_H */
