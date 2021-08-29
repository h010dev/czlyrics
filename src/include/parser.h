/**
 * @file   parser.h 
 * @author Mohamed Al-Hussein
 * @date   08/21/2021
 * @brief  Public header file for Parser.
 */
#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

/**
 * @brief Parsed endpoint.
 *
 * Includes artist name and song title.
 */
typedef struct
{
    char *artist;
    char *song;
} Endpoint;

/**
 * @brief Song data.
 *
 * Includes artist name, song title, and song lyrics.
 */
typedef struct
{
    char *artist_name;
    char *song_title;
    char *song_lyrics;
} SongData;

/**
 * @brief Parses URL and creates Endpoint struct with parsed data.
 *
 * Parses the request URL and fills out an Endpoint struct containing the artist name and song title.
 *
 * @param[in]      url      the URL to parse 
 * @param[in, out] endpoint the resulting Endpoint struct 
 * @param[out]     cz_errno error code signaling success (0) or failure (1)
 */
int parse_url (const char *url, Endpoint **endpoint);

/**
 * @brief Parses lyrics for given Endpoint and creates SongData struct with parsed data.
 *
 * Parses lyrics for given Endpoint and fills out a SongData struct containing the artist name, song tile,
 * and song lyrics.
 *
 * @param[in]      endpoint  Endpoint struct containing artist name and song title 
 * @param[in, out] song_data SongData struct containing artist name, song title, and song lyrics
 * @param[out]     cz_errno  error code signaling success (0) or failure (1)
 */
int parse_song_data (Endpoint *endpoint, SongData **song_data);

/**
 * @brief Checks if an html file for given endpoint exists.
 *
 * Creates a file name out of endpoint members and searches cache directory for matching html file.
 *
 * @param[in]      endpoint  Endpoint struct containing artist name and song title 
 * @param[out]     exists    true if file exists and false otherwise 
 */
bool file_exists (Endpoint *endpoint);

/**
 * @brief Frees Endpoint struct.
 *
 * Frees Endpoint struct and internal members (if need be).
 *
 * @param[in, out] endpoint Endpoint struct to be freed 
 */
void free_endpoint (Endpoint **endpoint);

/**
 * @brief Frees SongData struct.
 *
 * Frees SongData struct and internal members (if need be).
 *
 * @param[in, out] song_data SongData struct to be freed 
 */
void free_song_data (SongData **song_data);

#endif /* PARSER_H */
