// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// PLEASE READ BEFORE CHANGING THIS FILE!
//
// This file contains code that is used both inside and outside the out of
// bounds signal handler. Because this code runs in a signal handler context,
// use extra care when modifying this file. Here are some rules to follow.
//
// 1. Do not introduce any new external dependencies. This file needs
//    to be self contained so it is easy to audit everything that a
//    signal handler might do.
//
// 2. Any changes must be reviewed by someone from the crash reporting
//    or security team. See OWNERS for suggested reviewers.
//
// For more information, see https://goo.gl/yMeyUY.

#include "src/trap-handler/trap-handler-internal.h"

namespace v8 {
namespace internal {
namespace trap_handler {

// We declare this as int rather than bool as a workaround for a glibc bug, in
// which the dynamic loader cannot handle executables whose TLS area is only
// 1 byte in size; see https://sourceware.org/bugzilla/show_bug.cgi?id=14898.
THREAD_LOCAL int g_thread_in_wasm_code = false;

static_assert(sizeof(g_thread_in_wasm_code) > 1,
              "sizeof(thread_local_var) must be > 1, see "
              "https://sourceware.org/bugzilla/show_bug.cgi?id=14898");

size_t gNumCodeObjects = 0;
CodeProtectionInfoListEntry* gCodeObjects = nullptr;

#ifndef V8_OS_ZOS
std::atomic_flag MetadataLock::spinlock_ = ATOMIC_FLAG_INIT;
#endif

MetadataLock::MetadataLock() {
#ifndef V8_OS_ZOS
  if (g_thread_in_wasm_code) {
    abort();
  }

  while (spinlock_.test_and_set(std::memory_order::memory_order_acquire)) {
  }
#endif
}

MetadataLock::~MetadataLock() {
#ifndef V8_OS_ZOS
  if (g_thread_in_wasm_code) {
    abort();
  }

  spinlock_.clear(std::memory_order::memory_order_release);
#endif
}
}  // namespace trap_handler
}  // namespace internal
}  // namespace v8
