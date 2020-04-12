// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_
#define STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_

#include <stdint.h>

#include <vector>

#include "leveldb/slice.h"

namespace leveldb {

struct Options;

class BlockBuilder {
 public:
        // 调用者传进来的是 DBImpl 中的 options
        // options 提供了以下参数:
        //      block_restart_interval
        //      comparator  (从 DBImpl 调用来看，应该是 InternalKeyComparator )
  explicit BlockBuilder(const Options* options);

  BlockBuilder(const BlockBuilder&) = delete;
  BlockBuilder& operator=(const BlockBuilder&) = delete;

  // Reset the contents as if the BlockBuilder was just constructed.
  // TableBuilder::Flush() -> TableBuilder::WriteBlock() 处调用
        // TableBuilder 拥有一个 block_builder 对象，每次写满之后，会刷盘，然后调用 BlockBuilder::Reset()

  void Reset();

  // REQUIRES: Finish() has not been called since the last call to Reset().
  // REQUIRES: key is larger than any previously added key
  void Add(const Slice& key, const Slice& value);

  // Finish building the block and return a slice that refers to the
  // block contents.  The returned slice will remain valid for the
  // lifetime of this builder or until Reset() is called.
        // TableBuilder::WriteBlock() 刷盘之前，肯定要构建完整一个 block
        // ，添加对应的 restart[i] 和 num_restarts
        // 返回完整的 BlockBuilder::buffer_
  Slice Finish();

  // Returns an estimate of the current (uncompressed) size of the block
  // we are building.
        // 用于 TableBuilder::Add() 中判断是否该block已满，达到刷盘条件
  size_t CurrentSizeEstimate() const;

  // Return true iff no entries have been added since the last Reset()
  bool empty() const { return buffer_.empty(); }

 private:
        // options_->block_restart_interval 会针对 block 类型作更改
  const Options* options_;
  std::string buffer_;              // Destination buffer
  std::vector<uint32_t> restarts_;  // Restart points
  int counter_;                     // Number of entries emitted since restart
  bool finished_;                   // Has Finish() been called?
  std::string last_key_;
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_
