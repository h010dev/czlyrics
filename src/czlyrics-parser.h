#ifndef CZLYRICS_PARSER_H
#define CZLYRICS_PARSER_H

#include "stdlib.h"

extern const char* const FILENAME_FMT; /* HTML file save format string */

/* 
 * HTML markers
 *
 * Change these as necessary to keep up to date with target site's HTML formatting.
 */
extern const char* const ARTIST_NAME_START; /* String pattern marking start of artist name */
extern const char* const ARTIST_NAME_END;   /* String pattern marking end of artist name */
extern const char* const SONG_TITLE_START;  /* String pattern marking start of song title */
extern const char* const SONG_TITLE_END;    /* String pattern marking end of song title */
extern const char* const SONG_LYRICS_START; /* String pattern marking start of song lyrics */
extern const char* const SONG_LYRICS_END;   /* String pattern marking end of song lyrics */

struct Endpoint
{
    char *artist;
    char *song;
} Endpoint; /* endpoint split by artist name and song title */

struct SongData
{
    char *artist_name;
    char *song_title;
    char *song_lyrics;
} SongData; /* extracted song data */

/*
 * Parses the request URI and fills out an Endpoint struct containing the artist name
 * and song title.
 *
 * Returns 0 on success and 1 on failure.
 */
int parse_url (const char *url, struct Endpoint **endpoint);

/*
 * Parses a URL subdirectory and fills out a subdir string with non-alphanumeric characters removed.
 *
 * Returns pointer to URL after parsing string.
 */
char *parse_subdir (char *cur, char **subdir);

/*
 * Parses lyrics for given Endpoint and fills out a SongData struct containing the 
 * artist name, song tile, and song lyrics.
 *
 * Returns 0 on success and 1 on failure.
 */
int parse_song_data (struct Endpoint *endpoint, struct SongData **song_data);

/*
 * Parses HTML buffer and fills out the artist name.
 *
 * Returns the length of the artist name, which is 0 if not found or an error occurred.
 */
size_t parse_artist_name (char *textbuffer, char **artist_name);

/*
 * Parses HTML buffer and fills out the song title.
 *
 * Returns the length of the song title, which is 0 if not found or an error occurred.
 */
size_t parse_song_title (char *textbuffer, char **song_title);

/*
 * Parses HTML buffer and fills out the song lyrics.
 *
 * Returns the length of the song lyrics, which is 0 if not found or an error occurred.
 */
size_t parse_song_lyrics (char *textbuffer, char **song_lyrics);

/*
 * Loads HTML file into a char buffer.
 *
 * Returns 0 on success and 1 on failure.
 */
int buffer_file (struct Endpoint *endpoint, char **buffer);

/*
 * Given a start pattern and end pattern, slices string from HTML text buffer that
 * lies between the two.
 *
 * Returns the length of the slice, which is 0 if an error occurred.
 */
size_t slice_text (char *textbuffer, char *start_match, char *end_match, char **slice);

/*
 * Free memory for Endpoint.
 */
void free_endpoint (struct Endpoint **endpoint);

/*
 * Free memory for SongData.
 */
void free_song_data (struct SongData **song_data);

#endif /* CZLYRICS_PARSER_H */

