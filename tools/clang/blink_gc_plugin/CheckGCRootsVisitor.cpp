// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CheckGCRootsVisitor.h"
#include "BlinkGCPluginOptions.h"

CheckGCRootsVisitor::CheckGCRootsVisitor(const BlinkGCPluginOptions& options)
    : should_check_unique_ptrs_(options.enable_persistent_in_unique_ptr_check) {
}

CheckGCRootsVisitor::Errors& CheckGCRootsVisitor::gc_roots() {
  return gc_roots_;
}

bool CheckGCRootsVisitor::ContainsGCRoots(RecordInfo* info) {
  for (RecordInfo::Fields::iterator it = info->GetFields().begin();
       it != info->GetFields().end();
       ++it) {
    current_.push_back(&it->second);
    it->second.edge()->Accept(this);
    current_.pop_back();
  }
  return !gc_roots_.empty();
}

void CheckGCRootsVisitor::VisitValue(Value* edge) {
  // TODO: what should we do to check unions?
  if (edge->value()->record()->isUnion())
    return;

  // Prevent infinite regress for cyclic part objects.
  if (visiting_set_.find(edge->value()) != visiting_set_.end())
    return;

  visiting_set_.insert(edge->value());
  // If the value is a part object, then continue checking for roots.
  for (Context::iterator it = context().begin();
       it != context().end();
       ++it) {
    if (!(*it)->IsCollection())
      return;
  }
  ContainsGCRoots(edge->value());
  visiting_set_.erase(edge->value());
}

void CheckGCRootsVisitor::VisitUniquePtr(UniquePtr* edge) {
  if (!should_check_unique_ptrs_)
    return;
  edge->ptr()->Accept(this);
}

void CheckGCRootsVisitor::VisitPersistent(Persistent* edge) {
  gc_roots_.push_back(current_);
}
