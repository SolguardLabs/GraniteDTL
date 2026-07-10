#pragma once

#include "engine.hpp"

#include <map>
#include <string>
#include <vector>

namespace granite {

struct ExpiryBucket {
  TimePoint epoch = 0;
  std::int64_t positions = 0;
  Amount collateral = 0;
  Amount debt = 0;
  Amount requiredCollateral = 0;
  Amount surplus = 0;
  Amount shortfall = 0;
  Amount penaltyQuote = 0;
};

struct OwnerExposure {
  std::string account;
  std::int64_t activePositions = 0;
  std::int64_t expiredPositions = 0;
  std::int64_t terminalPositions = 0;
  Amount cash = 0;
  Amount collateral = 0;
  Amount debt = 0;
  Amount requiredCollateral = 0;
  Amount surplus = 0;
  Amount shortfall = 0;
  Amount posted = 0;
  Amount returned = 0;
  Amount penalties = 0;
};

struct PortfolioReport {
  std::vector<ExpiryBucket> buckets;
  std::vector<OwnerExposure> owners;
  TimePoint nextExpiry = 0;
  Amount unlockableNow = 0;
  Amount lockedUntilDue = 0;
  Amount projectedPenalty = 0;
  Amount projectedShortfall = 0;
  Amount maxBucketShortfall = 0;
  std::string riskiestOwner;
};

PortfolioReport buildPortfolio(const Engine& engine);

ExpiryBucket bucketForPosition(const Position& position,
                               const Policy& policy,
                               const LockBook& locks,
                               TimePoint now);

OwnerExposure exposureForAccount(const Account& account);

void mergePositionIntoExposure(OwnerExposure* exposure,
                               const Position& position,
                               const Policy& policy,
                               TimePoint now);

}  // namespace granite
