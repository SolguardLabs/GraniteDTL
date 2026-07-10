#pragma once

#include "types.hpp"

#include <string>
#include <vector>

namespace granite {

struct PenaltyQuote {
  bool applicable = false;
  bool capped = false;
  TimePoint lateness = 0;
  BasisPoints effectiveBps = 0;
  Amount targetPenalty = 0;
  Amount incrementalPenalty = 0;
};

struct LockQuote {
  bool open = false;
  bool expired = false;
  bool inGrace = false;
  TimePoint remaining = 0;
  TimePoint overdue = 0;
  TimePoint unlockEpoch = 0;
  std::string reason;
};

class LockBook {
 public:
  LockQuote quote(const Position& position, const Policy& policy, TimePoint now) const;

  PenaltyQuote quotePenalty(const Position& position,
                            const Policy& policy,
                            TimePoint now) const;

  bool shouldExpire(const Position& position, const Policy& policy, TimePoint now) const;

  bool canCompleteWithoutPenalty(const Position& position,
                                 const Policy& policy,
                                 TimePoint now) const;

  TimePoint nextUnlockEpoch(const Position& position, const Policy& policy) const;

  std::vector<std::string> explain(const Position& position,
                                   const Policy& policy,
                                   TimePoint now) const;
};

}  // namespace granite
