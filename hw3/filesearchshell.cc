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

#include <cstdlib>    // for EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>   // for std::cout, std::cerr, etc.
#include <cstring>    // for strlen
#include <cstdio>     // for getline
#include <sstream>    // for std::stringstream
#include <vector>     // for std::vector
#include <algorithm>  // for std::tolower
#include <list>       // for std::list

#include "./QueryProcessor.h"

using namespace std;
using namespace hw3;

// Error usage message for the client to see
// Arguments:
// - prog_name: Name of the program
static void Usage(char* prog_name);

// print the results of the query
// arguments:
// - result: the result vector to print
static void PrintOutput(vector<QueryProcessor::QueryResult> result);

// Your job is to implement the entire filesearchshell.cc
// functionality. We're essentially giving you a blank screen to work
// with; you need to figure out an appropriate design, to decompose
// the problem into multiple functions or classes if that will help,
// to pick good interfaces to those functions/classes, and to make
// sure that you don't leak any memory.
//
// Here are the requirements for a working solution:
//
// The user must be able to run the program using a command like:
//
//   ./filesearchshell ./foo.idx ./bar/baz.idx /tmp/blah.idx [etc]
//
// i.e., to pass a set of filenames of indices as command line
// arguments. Then, your program needs to implement a loop where
// each loop iteration it:
//
//  (a) prints to the console a prompt telling the user to input the
//      next query.
//
//  (b) reads a white-space separated list of query words from
//      std::cin, converts them to lowercase, and constructs
//      a vector of c++ strings out of them.
//
//  (c) uses QueryProcessor.cc/.h's QueryProcessor class to
//      process the query against the indices and get back a set of
//      query results.  Note that you should instantiate a single
//      QueryProcessor  object for the lifetime of the program, rather
//      than  instantiating a new one for every query.
//
//  (d) print the query results to std::cout in the format shown in
//      the transcript on the hw3 web page.
//
// Also, you're required to quit out of the loop when std::cin
// experiences EOF, which a user passes by pressing "control-D"
// on the console.  As well, users should be able to type in an
// arbitrarily long query -- you shouldn't assume anything about
// a maximum line length.  Finally, when you break out of the
// loop and quit the program, you need to make sure you deallocate
// all dynamically allocated memory.  We will be running valgrind
// on your filesearchshell implementation to verify there are no
// leaks or errors.
//
// You might find the following technique useful, but you aren't
// required to use it if you have a different way of getting the
// job done.  To split a std::string into a vector of words, you
// can use a std::stringstream to get the job done and the ">>"
// operator. See, for example, "gnomed"'s post on stackoverflow for
// their example on how to do this:
//
//   http://stackoverflow.com/questions/236129/c-how-to-split-a-string
//
// (Search for "gnomed" on that page. They use an istringstream, but
// a stringstream gets the job done too.)
//
// Good luck, and write beautiful code!
int main(int argc, char** argv) {
  if (argc < 2) {
    Usage(argv[0]);
  }

  // STEP 1:
  // Implement filesearchshell!
  // Probably want to write some helper methods ...

  // create index list
  list<string> index;
  for (int i = 1; i < argc; i++) {
    index.push_back(string(argv[i]));
  }

  QueryProcessor qp(index, true);
  while (1) {
    string input, word;
    vector<string> query;
    cout << "Enter query:" << endl;

    getline(cin, input);
    if (cin.eof() == 1 || input.empty()) break;

    // translate input to lowercase
    for (int i = 0; input[i]; i++)
      input[i] = tolower(input[i]);

    // split input into vector of words
    stringstream ss(input, stringstream::in);
    while (ss >> word) query.push_back(word);

    // process query
    vector<QueryProcessor::QueryResult> result =
      qp.ProcessQuery(query);

    PrintOutput(result);
  }

  return EXIT_SUCCESS;
}

static void Usage(char* prog_name) {
  cerr << "Usage: " << prog_name << " [index files+]" << endl;
  exit(EXIT_FAILURE);
}

static void PrintOutput(vector<QueryProcessor::QueryResult> result) {
  if (result.empty()) {
    cout << "  [no results]" << endl;
    return;
  }

  for (uint32_t i = 0; i < result.size(); i++) {
    cout << "  " << result[i].document_name <<
      " (" << result[i].rank << ")" << endl;
  }
}
