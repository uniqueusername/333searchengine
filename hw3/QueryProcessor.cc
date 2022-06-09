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

#include "./QueryProcessor.h"

#include <iostream>
#include <algorithm>
#include <list>
#include <string>
#include <vector>

extern "C" {
  #include "./libhw1/CSE333.h"
}

using std::list;
using std::sort;
using std::string;
using std::vector;

namespace hw3 {

QueryProcessor::QueryProcessor(const list<string>& index_list, bool validate) {
  // Stash away a copy of the index list.
  index_list_ = index_list;
  array_len_ = index_list_.size();
  Verify333(array_len_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader* [array_len_];
  itr_array_ = new IndexTableReader* [array_len_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::const_iterator idx_iterator = index_list_.begin();
  for (int i = 0; i < array_len_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = fir.NewDocTableReader();
    itr_array_[i] = fir.NewIndexTableReader();
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (int i = 0; i < array_len_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

// This structure is used to store a index-file-specific query result.
typedef struct {
  DocID_t doc_id;  // The document ID within the index file.
  int     rank;    // The rank of the result so far.
} IdxQueryResult;

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string>& query) const {
  Verify333(query.size() > 0);

  // STEP 1.
  // (the only step in this file)
  vector<QueryProcessor::QueryResult> final_result;

  for (uint32_t i = 0; i < query.size(); i++) {
    vector<QueryProcessor::QueryResult> init_result = final_result;

    for (int j = 0; j < array_len_; j++) {
      // setup query lookup
      IndexTableReader *itr = itr_array_[j];
      DocTableReader *dtr = dtr_array_[j];
      DocIDTableReader *ditr = itr->LookupWord(query[i]);

      // no corresponding results found
      if (ditr == NULL) continue;

      // otherwise obtain list of doc ids
      list<DocIDElementHeader> doc_list = ditr->GetDocIDList();

      // iterate through headers
      for (auto element : doc_list) {
        string doc;
        Verify333(dtr->LookupDocID(element.doc_id, &doc));

        // create result
        QueryProcessor::QueryResult result;
        result.document_name = doc;
        result.rank = element.num_positions;

        if (i == 0) {
          // for first encounter of query
          final_result.push_back(result);
        } else {
          // otherwise increase rank
          for (uint32_t k = 0; k < final_result.size(); k++) {
            if ((final_result[k].document_name)
              .compare(result.document_name) == 0)
              final_result[k].rank += result.rank;
          }
        }
      }
      delete ditr;
    }

    // remove duplicates
    for (QueryProcessor::QueryResult result : init_result) {
      for (uint32_t i = 0; i < final_result.size(); i++) {
        if ((final_result[i].document_name)
          .compare(result.document_name) == 0 &&
          (final_result[i].rank == result.rank))
            final_result.erase(final_result.begin() + i);
      }
    }
  }

  // Sort the final results.
  sort(final_result.begin(), final_result.end());
  return final_result;
}

}  // namespace hw3
