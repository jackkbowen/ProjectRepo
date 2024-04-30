#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cache.h"
#include <bits/getopt_core.h>

#define DEFAULT_CACHE_SIZE 1
#define BUFFER_SIZE        100

int main(int argc, char **argv) {
    char *policy = "-F";
    int cacheSize = DEFAULT_CACHE_SIZE;
    int opt;
    char buffer[BUFFER_SIZE];

    // Check if there is the right parameters
    if (argc < 2) {
        fprintf(stderr, "usage: %s [-N size] <policy>\n", argv[0]);
        return EXIT_FAILURE;
    } else if (argc == 2) {
        policy = argv[1];
    }
    //printf("Num of args: %i\n", argc);

    while ((opt = getopt(argc, argv, ":N:F:L:C:")) != -1) {
        switch (opt) {
        case 'N':
            cacheSize = atoi(optarg);
            policy = argv[3];
            break;
        }
    }
    Cache *cache = cache_new(cacheSize, policy);
    //fprintf(stdout, "Cache Size = %i\nPolicy Type: %s\n", cacheSize, policy);

    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        // logic to determine hit or miss
        insert(cache, buffer);
    }

    int capMisses = getcapMisses(cache);
    int compMisses = getcompMisses(cache);
    fprintf(stdout, "%d %d\n", compMisses, capMisses);

    return 0;
}
