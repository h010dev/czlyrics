#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"

#include "czlyrics-parser.h"

const char* const FILENAME_FMT      = "./cache/%s_%s.html";
const char* const ARTIST_NAME_START = "ArtistName = \"";
const char* const ARTIST_NAME_END   = "\"";
const char* const SONG_TITLE_START  = "SongName = \"";
const char* const SONG_TITLE_END    = "\"";
const char* const SONG_LYRICS_START = "Sorry about that. -->";
const char* const SONG_LYRICS_END   = "</div>";

int
parse_url (const char *s_url, struct Endpoint **endpoint)
{
    char *artist, *song, *match, *cur;

    // Seek past API endpoint
    match = "/api/lyrics/"; 
    cur = strstr (s_url, match);
    cur += strlen (match);

    cur = parse_subdir (cur, &artist);
    (*endpoint)->artist = strdup (artist);
    free (artist);

    (void) parse_subdir (++cur, &song);
    (*endpoint)->song = strdup (song);
    free (song);

    return 0;
}

char *
parse_subdir (char *cur, char **subdir)
{
    int  pos;
    char temp[255];

    pos = 0;
    while (*cur != '/')
    {
        if (isalnum (*cur))
        {
            temp[pos] = tolower (*cur);
            pos++;
        }
        else if (*cur == '%') /* % marks beginning of space (%20) */
            cur += 2;
        cur++;
    }
    temp[pos] = '\0';
    *subdir = strdup (temp);

    return cur;
}

int
parse_song_data (struct Endpoint *endpoint, struct SongData **song_data)
{
    char  *f_data;
    int    cz_errno;
    size_t len;

    // Load HTML file into buffer
    if ( (cz_errno = buffer_file (endpoint, &f_data)) != 0 )
        return 1;

    // Extract artist name
    char *s_artist_name;
    if ( (len = parse_artist_name (f_data, &s_artist_name)) == 0 )
    {
        free (f_data);
        printf ("Failed to allocate space for artist name.\n");
        return 1;
    }
    (*song_data)->artist_name = s_artist_name;

    // Extract song title
    char *s_song_title;
    if ( (len = parse_song_title (f_data, &s_song_title)) == 0 )
    {
        free (f_data);
        printf ("Failed to allocate space for song title.\n");
        return 1;
    }
    (*song_data)->song_title = s_song_title;

    // Extract and clean song lyrics
    char *s_song_lyrics;
    if ( (len = parse_song_lyrics (f_data, &s_song_lyrics)) == 0 )
    {
        free (f_data);
        printf ("Failed to allocate space for song lyrics.\n");
        return 1;
    }
    (*song_data)->song_lyrics = s_song_lyrics;

    free (f_data);

    return 0;
}

int
buffer_file (struct Endpoint *endpoint, char **buffer)
{
    // Open html file 
    FILE *fp;
    char  f_path[1024];

    snprintf (f_path, sizeof (f_path), FILENAME_FMT, endpoint->artist, endpoint->song);
    if ( (fp = fopen (f_path, "rb")) == NULL)
    {
        printf ("File couldn't be opened.\n");
        return 1;
    }

    // Allocate buffer for file contents
    long  f_size;

    fseek (fp, 0L, SEEK_END);
    f_size = ftell (fp);
    rewind (fp);

    if ( (*buffer = calloc (1, f_size + 1)) == NULL)
    {
        fclose (fp);
        free (*buffer);
        printf ("Failed to allocate space for file.\n");
        return 1;
    }

    // Read data from file into file buffer
    if (fread (*buffer, f_size, 1, fp) != 1)
    {
        fclose (fp);
        free (*buffer);
        printf ("File could not be read.\n");
        return 1;
    }
    fclose (fp);

    return 0;
}

size_t
slice_text (char *textbuffer, char *start_match, char *end_match, char **slice)
{
    char  *temp;
    char  *cur, *cur_end;
    size_t len;

    cur = strstr (textbuffer, start_match);
    cur += strlen (start_match);
    cur_end = strstr (cur, end_match);
    cur_end--;

    len = (char *) cur_end - (char *) cur + 1;

    if ( (*slice = calloc (1, len)) == NULL )
        return 0;

    memcpy (*slice, cur, len);

    return len;
}

size_t
parse_artist_name (char *textbuffer, char **artist_name)
{
    int    pos;
    size_t len;

    if ( (len = slice_text (textbuffer, (char *) ARTIST_NAME_START, (char *) ARTIST_NAME_END, &*artist_name)) == 0 )
        return 0;

    return len;
}

size_t
parse_song_title (char *textbuffer, char **song_title)
{
    int    pos;
    size_t len;

    if ( (len = slice_text (textbuffer, (char *) SONG_TITLE_START, (char *) SONG_TITLE_END, &*song_title)) == 0 )
        return 0;

    return len;
}

size_t
parse_song_lyrics (char *textbuffer, char **song_lyrics)
{
    char  *cur, *s_song_lyrics, *s_raw_lyrics;
    int    pos;
    size_t len;

    if ( (len = slice_text (textbuffer, (char *) SONG_LYRICS_START, (char *) SONG_LYRICS_END, &s_raw_lyrics)) == 0 )
        return 0;

    s_song_lyrics = calloc (len, sizeof (char));

    // Scan lyrics and replace any tags
    pos = 0;
    cur = s_raw_lyrics;
    while (*cur != '\0')
    {
        if (*cur == '<')              /* start of a tag */
        {
            while (*cur != '>')       /* seek to end of tag */
                cur++;
            cur++;                    /* seek past closing tag */
            continue;
        }
        else if (*cur == '&')         /* start of an html entity/symbol */
        {
            while (*cur != ';')       /* seek to end of symbol */
                cur++;
            cur++;                    /* seek past semicolon */
            continue;
        }
        else
        {
            s_song_lyrics[pos] = cur[0];
            cur++;
        }
        pos++;
    }
    *song_lyrics = strdup (s_song_lyrics);
    free (s_song_lyrics);

    return len;
}

void
free_endpoint (struct Endpoint **endpoint)
{
    free ((*endpoint)->song);
    free ((*endpoint)->artist);
    free (*endpoint);
}

void
free_song_data (struct SongData **song_data)
{
    free ((*song_data)->song_lyrics);
    free ((*song_data)->song_title);
    free ((*song_data)->artist_name);
    free (*song_data);
}

