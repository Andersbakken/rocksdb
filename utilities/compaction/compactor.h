// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
//
// This file contains the interface for implementing custom background
// compaction strategy.

#pragma once
#include <set>
#include <vector>

#include "rocksdb/options.h"
#include "rocksdb/status.h"
#include "rocksdb/metadata.h"

namespace rocksdb {
struct Options;

// The pluggable component to PluggableCompactionPicker that allows
// developers to write their own compaction strategies.  It's currently
// a dummy class and will move to include/rocksdb/db.h once its API
// is completed.
//
// Note that all functions must not run in an extended period of time.
// Otherwise, RocksDB may be blocked by these function calls.
class Compactor {
 public:
  // Constructs a compactor given the currently used options.
  explicit Compactor(const ImmutableCFOptions& ioptions,
                     const CompactionOptions& coptions) :
      ioptions_(ioptions), compact_options_(coptions) {}

  // Given the meta data describes the current state of a column
  // family, this function will determine a list of compaction
  // input files and output level if the input column family
  // underlies a good compaction job.  If a non-ok status is
  // returned (in such case it's usually Status::NotFound()),
  // it means the input column family does not underlies a good
  // compaction job.
  //
  // If output_level is set to kDeletionCompaction, then it will
  // simply delete the selected files.
  virtual Status PickCompaction(
      std::vector<uint64_t>* input_file_numbers, int* output_level,
      const ColumnFamilyMetaData& cf_meta) = 0;

  // Similar to PickCompaction, but with one requirement that the
  // resulting list of compaction input files must be in the
  // specified "input_level" and the compaction output level must
  // be "output_level".
  virtual Status PickCompactionByRange(
      std::vector<uint64_t>* input_file_Numbers,
      const ColumnFamilyMetaData& cf_meta,
      const int input_level, const int output_level) = 0;

  // Sanitize the compaction input files "input_files" to a valid
  // one by adding more files to it.  A non-ok status is returned
  // if the input cannot be adjusted to be a valid compaction.
  virtual Status SanitizeCompactionInputFiles(
      std::set<uint64_t>* input_files,
      const ColumnFamilyMetaData& cf_meta,
      const int output_level) = 0;

  const CompactionOptions& compact_options() {
    return compact_options_;
  }

 protected:
  const ImmutableCFOptions& ioptions_;
  const CompactionOptions& compact_options_;
};

class CompactorFactory {
 public:
  explicit CompactorFactory(const CompactionOptions& coptions)
      : compact_options_(coptions) {}

  // Creates a pointer to a Compactor object.
  virtual Compactor* CreateCompactor(
      const ImmutableCFOptions& options) = 0;

  const CompactionOptions compact_options_;
};

}  // namespace rocksdb
