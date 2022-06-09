/*
 * Copyright ©2022 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2022 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

// copyright 2022 ary jha
// aryanjha@uw.edu <1902079>

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600
#define BUF_SIZE 1024
#define MAX_TOKENS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "./CrawlFileTree.h"
#include "./DocTable.h"
#include "./MemIndex.h"

//////////////////////////////////////////////////////////////////////////////
// Helper function declarations, constants, etc
static void Usage(void);
static void ProcessQueries(DocTable* dt, MemIndex* mi);

//////////////////////////////////////////////////////////////////////////////
// Main
int main(int argc, char** argv) {
  if (argc != 2) {
    Usage();
  }

  // Implement searchshell!  We're giving you very few hints
  // on how to do it, so you'll need to figure out an appropriate
  // decomposition into functions as well as implementing the
  // functions.  There are several major tasks you need to build:
  //
  //  - Crawl from a directory provided by argv[1] to produce and index
  //  - Prompt the user for a query and read the query from stdin, in a loop
  //  - Split a query into words (check out strtok_r)
  //  - Process a query against the index and print out the results
  //
  // When searchshell detects end-of-file on stdin (cntrl-D from the
  // keyboard), searchshell should free all dynamically allocated
  // memory and any other allocated resources and then exit.
  //
  // Note that you should make sure the fomatting of your
  // searchshell output exactly matches our solution binaries
  // to get full points on this part.

  DocTable *dt;
  MemIndex *mi;

  // index directory
  printf("Indexing \'%s\'\n", argv[1]);
  if (!CrawlFileTree(argv[1], &dt, &mi)) {
      Usage();
      return EXIT_FAILURE;
  }

  Verify333(dt != NULL);
  Verify333(mi != NULL);

  // process queried words
  ProcessQueries(dt, mi);

  DocTable_Free(dt);
  MemIndex_Free(mi);

  return EXIT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
// Helper function definitions

static void Usage(void) {
    fprintf(stderr, "Usage: ./searchshell <docroot>\n");
    fprintf(stderr,
            "where <docroot> is an absolute or relative " \
            "path to a directory to build an index under.\n");
    exit(EXIT_FAILURE);
}

static void ProcessQueries(DocTable* dt, MemIndex* mi) {
    char buf[BUF_SIZE];

    // take inputs until program terminates
    while (1) {
        printf("enter query:\n");

        // terminate on null input
        if (fgets(buf, BUF_SIZE, stdin) == NULL) { break; }

        // clean input
        buf[strlen(buf) - 1] = '\0';

        // process queries by token
        char **queries = (char **) malloc(MAX_TOKENS * sizeof(char *));
        char *token;
        char *curr = buf;
        int q_index = 0;

        while ((token = strtok_r(curr, " ", &curr)) != NULL) {
            queries[q_index] = token;
            q_index++;
        }

        // print results
        LinkedList *results = MemIndex_Search(mi, queries, q_index);
        if (results == NULL) {
            // continue when out of results
            free(queries);
            continue;
        } else {
            LLIterator *itr = LLIterator_Allocate(results);
            while (LLIterator_IsValid(itr)) {
                SearchResult *result;
                LLIterator_Get(itr, (LLPayload_t *) &result);
                char* file_name = DocTable_GetDocName(dt, result->doc_id);
                printf("  %s (%d)\n", file_name, result->rank);
                LLIterator_Next(itr);
            }

            LLIterator_Free(itr);
        }

        free(queries);
        LinkedList_Free(results, &free);
    }
}
