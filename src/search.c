#include "search.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int safe_copy(char *dst, size_t capacity, const char *src) {
    size_t length;
    if (dst == NULL || src == NULL || capacity == 0) return -1;
    length = strlen(src);
    if (length == 0 || length >= capacity) return -1;
    memcpy(dst, src, length + 1);
    return 0;
}

static int id_is_valid(const char *id) {
    const unsigned char *cursor = (const unsigned char *)id;
    size_t length;
    if (id == NULL) return 0;
    length = strlen(id);
    if (length == 0 || length >= SKY_SEARCH_MAX_ID) return 0;
    while (*cursor != '\0') {
        if (!(isalnum(*cursor) || *cursor == '-' || *cursor == '_' || *cursor == '.' || *cursor == ':')) return 0;
        cursor++;
    }
    return 1;
}

static unsigned int token_occurrences(const char *text, const char *token) {
    char word[128];
    size_t word_length = 0;
    unsigned int count = 0;
    const unsigned char *cursor = (const unsigned char *)text;

    while (1) {
        if (isalnum(*cursor)) {
            if (word_length + 1 < sizeof(word)) word[word_length++] = (char)tolower(*cursor);
        } else if (word_length > 0) {
            word[word_length] = '\0';
            if (strcmp(word, token) == 0) count++;
            word_length = 0;
        }
        if (*cursor == '\0') break;
        cursor++;
    }
    return count;
}

static size_t parse_query_terms(const char *query, char terms[][128], size_t max_terms) {
    size_t count = 0;
    size_t length = 0;
    const unsigned char *cursor = (const unsigned char *)query;

    while (*cursor != '\0' && count < max_terms) {
        if (isalnum(*cursor)) {
            if (length + 1 < 128) terms[count][length++] = (char)tolower(*cursor);
        } else if (length > 0) {
            terms[count][length] = '\0';
            count++;
            length = 0;
        }
        cursor++;
    }
    if (length > 0 && count < max_terms) {
        terms[count][length] = '\0';
        count++;
    }
    return count;
}

static void insert_result(SkySearchResult *results, size_t *count, size_t capacity, const SkySearchResult *candidate) {
    size_t position;
    if (capacity == 0) return;
    if (*count < capacity) {
        position = (*count)++;
    } else {
        if (candidate->score <= results[capacity - 1].score) return;
        position = capacity - 1;
    }

    while (position > 0 && candidate->score > results[position - 1].score) {
        if (position < capacity) results[position] = results[position - 1];
        position--;
    }
    results[position] = *candidate;
}

void sky_search_init(SkySearchIndex *index) {
    if (index != NULL) memset(index, 0, sizeof(*index));
}

int sky_search_add(SkySearchIndex *index, const char *id, const char *title, const char *body) {
    size_t i;
    SkyDocument document;
    if (index == NULL || !id_is_valid(id) || title == NULL || body == NULL) return -1;
    if (index->count >= SKY_SEARCH_MAX_DOCS) return -2;
    for (i = 0; i < index->count; i++) {
        if (strcmp(index->documents[i].id, id) == 0) return -3;
    }
    memset(&document, 0, sizeof(document));
    if (safe_copy(document.id, sizeof(document.id), id) != 0) return -1;
    if (safe_copy(document.title, sizeof(document.title), title) != 0) return -1;
    if (safe_copy(document.body, sizeof(document.body), body) != 0) return -1;
    index->documents[index->count++] = document;
    return 0;
}

size_t sky_search_query(const SkySearchIndex *index, const char *query, SkySearchResult *results, size_t result_capacity) {
    char terms[32][128] = {{0}};
    size_t term_count;
    size_t i;
    size_t result_count = 0;
    if (index == NULL || query == NULL || results == NULL || result_capacity == 0) return 0;
    if (strlen(query) == 0 || strlen(query) >= SKY_SEARCH_MAX_QUERY) return 0;
    if (result_capacity > SKY_SEARCH_MAX_RESULTS) result_capacity = SKY_SEARCH_MAX_RESULTS;
    term_count = parse_query_terms(query, terms, 32);
    if (term_count == 0) return 0;

    for (i = 0; i < index->count; i++) {
        size_t term;
        unsigned int score = 0;
        int all_terms = 1;
        for (term = 0; term < term_count; term++) {
            unsigned int title_hits = token_occurrences(index->documents[i].title, terms[term]);
            unsigned int body_hits = token_occurrences(index->documents[i].body, terms[term]);
            if (title_hits + body_hits == 0) {
                all_terms = 0;
                break;
            }
            score += title_hits * 3U + body_hits;
        }
        if (all_terms) {
            SkySearchResult candidate;
            memset(&candidate, 0, sizeof(candidate));
            (void)safe_copy(candidate.id, sizeof(candidate.id), index->documents[i].id);
            (void)safe_copy(candidate.title, sizeof(candidate.title), index->documents[i].title);
            candidate.score = score;
            insert_result(results, &result_count, result_capacity, &candidate);
        }
    }
    return result_count;
}

int sky_search_load_tsv(SkySearchIndex *index, const char *path) {
    char line[SKY_SEARCH_MAX_ID + SKY_SEARCH_MAX_TITLE + SKY_SEARCH_MAX_BODY + 8];
    FILE *file;
    if (index == NULL || path == NULL) return -1;
    file = fopen(path, "r");
    if (file == NULL) return -1;
    while (fgets(line, sizeof(line), file) != NULL) {
        char *id = line;
        char *title = strchr(id, '\t');
        char *body;
        char *newline;
        int result;
        if (title == NULL) { fclose(file); return -2; }
        *title++ = '\0';
        body = strchr(title, '\t');
        if (body == NULL) { fclose(file); return -2; }
        *body++ = '\0';
        newline = strpbrk(body, "\r\n");
        if (newline != NULL) *newline = '\0';
        result = sky_search_add(index, id, title, body);
        if (result != 0) { fclose(file); return result; }
    }
    if (ferror(file)) { fclose(file); return -1; }
    fclose(file);
    return 0;
}
