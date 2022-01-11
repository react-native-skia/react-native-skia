/*
* Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#pragma once

#include "include/core/SkPoint.h"

#include "RSkSpatialNavigator.h"

using namespace facebook::react;

namespace SpatialNavigator {

enum ScrollStatus {
  noScroll = 1,
  scrollOnly,
  scrollToFocus
};

class Container {
private:

protected:
  // List of Navigatable Components in the container
  CandidateList navComponentList_;

public:
  friend class RSkComponent;
  Container() {};
  ~Container() {};

  virtual bool canScrollInDirection(rnsKey direction) { return false;}
  virtual ScrollStatus scrollInDirection(RSkComponent* candidate, rnsKey direction) { return noScroll;}
  virtual bool isVisible(RSkComponent* candidate) { return false;}
  virtual bool isScrollable() { return false;}
  virtual SkPoint getScrollOffset() {return SkPoint::Make(0,0);}

  void addComponent(RSkComponent *candidate);
  void mergeComponent(CandidateList candidates);
  void removeComponent(RSkComponent *candidate);
  void updateComponent(RSkComponent *candidate);

  CandidateList& navigationCandidates() { return navComponentList_; }

  RSkComponent* firstInContainer(bool visible = true, bool skipChildren = false);
  RSkComponent* preferredFocusInContainer(); // Find the default preferred focus in the container
};

} // namespace SpatialNavigator
