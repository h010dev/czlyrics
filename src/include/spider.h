/**
 * @file   spider.h 
 * @author Mohamed Al-Hussein
 * @date   08/21/2021
 * @brief  Public header file for Spider.
 */
#ifndef SPIDER_H
#define SPIDER_H

/**
 * @brief Scrapes lyrics for matching artist and song from target site.
 *
 * Given an artist and song, attempts to scrape lyrics off target site with matching values.
 *   If anything goes wrong (e.g. lyrics not found), returns 1.
 *   Otherwise, returns 0.
 *
 * @param[in]      artist the artist name 
 * @param[in, out] song   the song title 
 * @param[out]     cz_errno error code signaling success (0) or failure (1)
 */
int scrape_lyrics (const char *artist, const char *song);

#endif /* SPIDER_H */
