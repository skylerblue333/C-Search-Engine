#ifndef SKY_SEARCH_H
#define SKY_SEARCH_H

#include <stddef.h>

#define SKY_SEARCH_MAX_DOCS 4096
#define SKY_SEARCH_MAX_ID 64
#define SKY_SEARCH_MAX_TITLE 256
#define SKY_SEARCH_MAX_BODY 4096
#define SKY_SEARCH_MAX_QUERY 512
#define SKY_SEARCH_MAX_RESULTS 100

typedef struct {
    char id[SKY_SEARCH_MAX_ID];
    char title[SKY_SEARCH_MAX_TITLE];
    char body[SKY_SEARCH_MAX_BODY];
} SkyDocument;

typedef struct {
    char id[SKY_SEARCH_MAX_ID];
    char title[SKY_SEARCH_MAX_TITLE];
    unsigned int score;
} SkySearchResult;

typedef struct {
    SkyDocument documents[SKY_SEARCH_MAX_DOCS];
    size_t count;
} SkySearchIndex;

void sky_search_init(SkySearchIndex *index);
int sky_search_add(SkySearchIndex *index, const char *id, const char *title, const char *body);
size_t sky_search_query(
    const SkySearchIndex *index,
    const char *query,
    SkySearchResult *results,
    size_t result_capacity
);
int sky_search_load_tsv(SkySearchIndex *index, const char *path);

#endif
