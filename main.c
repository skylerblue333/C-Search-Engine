#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DOCS 100
#define MAX_WORDS 1000
#define MAX_LEN 256

typedef struct {
    int doc_id;
    char word[MAX_LEN];
} IndexEntry;

IndexEntry index[MAX_WORDS];
int index_size = 0;

const char *documents[] = {
    "the quick brown fox jumps over the lazy dog",
    "a fast red fox ran through the green forest",
    "the lazy dog slept all day in the sun"
};

void build_index(int num_docs) {
    for (int i = 0; i < num_docs; i++) {
        char buf[512];
        strncpy(buf, documents[i], sizeof(buf) - 1);
        char *token = strtok(buf, " ");
        while (token && index_size < MAX_WORDS) {
            strncpy(index[index_size].word, token, MAX_LEN - 1);
            index[index_size].doc_id = i;
            index_size++;
            token = strtok(NULL, " ");
        }
    }
}

void search(const char *query) {
    printf("Search results for \"%s\":\n", query);
    int found = 0;
    for (int i = 0; i < index_size; i++) {
        if (strcmp(index[i].word, query) == 0) {
            printf("  -> Document %d: \"%s\"\n", index[i].doc_id, documents[index[i].doc_id]);
            found++;
        }
    }
    if (!found) printf("  No results found.\n");
}

int main() {
    int num_docs = sizeof(documents) / sizeof(documents[0]);
    build_index(num_docs);
    printf("Index built: %d entries across %d documents.\n\n", index_size, num_docs);
    search("fox");
    search("lazy");
    search("cloud");
    return 0;
}
