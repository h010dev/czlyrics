/**
 * @file   parser-internal.h 
 * @author Mohamed Al-Hussein
 * @date   08/21/2021
 * @brief  Internal header file for Parser.
 */
#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include "include/parser.h"

#include "stdlib.h"

extern const char* const FILENAME_FMT; /**< HTML file save format string */

/* 
 * HTML markers
 *
 * Change these as necessary to keep up to date with target site's HTML formatting.
 */
extern const char* const ARTIST_NAME_START; /**< String pattern marking start of artist name */
extern const char* const ARTIST_NAME_END;   /**< String pattern marking end of artist name */
extern const char* const SONG_TITLE_START;  /**< String pattern marking start of song title */
extern const char* const SONG_TITLE_END;    /**< String pattern marking end of song title */
extern const char* const SONG_LYRICS_START; /**< String pattern marking start of song lyrics */
extern const char* const SONG_LYRICS_END;   /**< String pattern marking end of song lyrics */

/**
 * @brief Parses a URL subdirectory and creates subdir string with cleaned data.
 *
 * Parses a URL subdirectory and fills out a subdir string with non-alphanumeric characters removed.
 *
 * @param[in]      cur    pointer to specific position within URL string 
 * @param[in, out] subdir the resulting subdirectory 
 * @param[out]     pos    points to next subdirectory within URL string (if any) 
 */
static char *parse_subdir (char *cur, char **subdir);

/**
 * @brief Parses HTML text buffer and creates string containing artist name.
 *
 * Searches HTML text buffer for artist name. 
 *   If nothing is found (or an error occurred), returns 0.
 *   Otherwise, returns length of the artist name.
 *
 * @param[in]      textbuffer  HTML text buffer to search within 
 * @param[in, out] artist_name artist name (if found) 
 * @param[out]     len         artist name length 
 */
static size_t parse_artist_name (char *textbuffer, char **artist_name);

/**
 * @brief Parses HTML text buffer and creates string containing song title.
 *
 * Searches HTML text buffer for song title. 
 *   If nothing is found (or an error occurred), returns 0.
 *   Otherwise, returns length of the song title.
 *
 * @param[in]      textbuffer HTML text buffer to search within 
 * @param[in, out] song_title song title (if found) 
 * @param[out]     len        song title length 
 */
static size_t parse_song_title (char *textbuffer, char **song_title);

/**
 * @brief Parses HTML text buffer and creates string containing song lyrics.
 *
 * Searches HTML text buffer for song lyrics. 
 *   If nothing is found (or an error occurred), returns 0.
 *   Otherwise, returns length of the song lyrics.
 *
 * @param[in]      textbuffer  HTML text buffer to search within 
 * @param[in, out] song_lyrics song lyrics (if found) 
 * @param[out]     len         song lyrics length 
 */
static size_t parse_song_lyrics (char *textbuffer, char **song_lyrics);

/**
 * @brief Reads HTML file matching endpoint values and fills out buffer with entire file.
 *
 * Given an Endpoint struct, searches cache for HTML file with matching artist name and song title.
 *   If found, attempts to read contents into @p buffer. 
 *   If not found, or if an error occurred, returns 1.
 *   Otherwise, returns 0 to signal success.
 *
 * @param[in]      endpoint Endpoint struct containing artist name and song title to search for 
 * @param[in, out] buffer   buffer to hold entire HTML contents 
 * @param[out]     cz_errno error code signaling success (0) or failure (1)      
 */
static int buffer_file (struct Endpoint *endpoint, char **buffer);

/**
 * @brief Creates @p slice holding text from @p textbuffer that lies between @p start_match and @p end_match.
 *
 * Given a start pattern and end pattern, slices string from HTML text buffer that lies between the two.
 *   If nothing is found (or an error occurred), returns 0.
 *   Otherwise, returns length of slice.
 *   
 * @param[in]      textbuffer  HTML text buffer to search within 
 * @param[in]      start_match string pattern to look for (first instance) that denotes start of slice 
 * @param[out]     end_match   string pattern to look for (first instance) that denotes end of slice      
 * @param[in, out] slice       string containing result (if any)
 */
static size_t slice_text (char *textbuffer, char *start_match, char *end_match, char **slice);

#endif /* PARSER_INTERNAL_H */
