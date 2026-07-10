#include "portfolio.hpp"

#include <algorithm>

namespace granite {

namespace {

OwnerExposure* ensureOwner(std::map<std::string, OwnerExposure>* owners,
                           const Account& account) {
  OwnerExposure& exposure = (*owners)[account.id];
  if (exposure.account.empty()) {
    exposure = exposureForAccount(account);
  }
  return &exposure;
}

ExpiryBucket* ensureBucket(std::map<TimePoint, ExpiryBucket>* buckets, TimePoint epoch) {
  ExpiryBucket& bucket = (*buckets)[epoch];
  if (bucket.epoch == 0) {
    bucket.epoch = epoch;
  }
  return &bucket;
}

void mergeBucket(ExpiryBucket* bucket, const ExpiryBucket& positionBucket) {
  if (bucket == nullptr) {
    return;
  }

  bucket->positions += positionBucket.positions;
  bucket->collateral += positionBucket.collateral;
  bucket->debt += positionBucket.debt;
  bucket->requiredCollateral += positionBucket.requiredCollateral;
  bucket->surplus += positionBucket.surplus;
  bucket->shortfall += positionBucket.shortfall;
  bucket->penaltyQuote += positionBucket.penaltyQuote;
}

}  // namespace

PortfolioReport buildPortfolio(const Engine& engine) {
  PortfolioReport report;
  std::map<TimePoint, ExpiryBucket> buckets;
  std::map<std::string, OwnerExposure> owners;
  LockBook locks;

  for (const auto& accountItem : engine.accounts()) {
    ensureOwner(&owners, accountItem.second);
  }

  for (const auto& positionItem : engine.positions()) {
    const Position& position = positionItem.second;
    const ExpiryBucket positionBucket =
        bucketForPosition(position, engine.policy(), locks, engine.now());
    ExpiryBucket* bucket = ensureBucket(&buckets, positionBucket.epoch);
    mergeBucket(bucket, positionBucket);

    const auto accountFound = engine.accounts().find(position.owner);
    if (accountFound != engine.accounts().end()) {
      OwnerExposure* exposure = ensureOwner(&owners, accountFound->second);
      mergePositionIntoExposure(exposure, position, engine.policy(), engine.now());
    }

    if (!position.terminal) {
      if (report.nextExpiry == 0 || position.dueAt < report.nextExpiry) {
        report.nextExpiry = position.dueAt;
      }

      if (position.dueAt <= engine.now()) {
        report.unlockableNow += position.lastQuotedSurplus;
      } else {
        report.lockedUntilDue += position.collateral;
      }
    }
  }

  for (const auto& item : buckets) {
    report.buckets.push_back(item.second);
    report.projectedPenalty += item.second.penaltyQuote;
    report.projectedShortfall += item.second.shortfall;
    if (item.second.shortfall > report.maxBucketShortfall) {
      report.maxBucketShortfall = item.second.shortfall;
    }
  }

  Amount largestOwnerShortfall = 0;
  for (const auto& item : owners) {
    report.owners.push_back(item.second);
    if (item.second.shortfall > largestOwnerShortfall) {
      largestOwnerShortfall = item.second.shortfall;
      report.riskiestOwner = item.second.account;
    }
  }

  return report;
}

ExpiryBucket bucketForPosition(const Position& position,
                               const Policy& policy,
                               const LockBook& locks,
                               TimePoint now) {
  ExpiryBucket bucket;
  bucket.epoch = position.dueAt;
  bucket.positions = 1;
  bucket.collateral = position.collateral;
  bucket.debt = position.debt;
  bucket.requiredCollateral = requiredCollateralFor(position.debt, policy.minCoverageBps);
  bucket.surplus = quoteSurplus(position.collateral, bucket.requiredCollateral);
  bucket.shortfall = quoteShortfall(position.collateral, bucket.requiredCollateral);
  bucket.penaltyQuote = locks.quotePenalty(position, policy, now).incrementalPenalty;
  return bucket;
}

OwnerExposure exposureForAccount(const Account& account) {
  OwnerExposure exposure;
  exposure.account = account.id;
  exposure.cash = account.cash;
  exposure.posted = account.collateralPosted;
  exposure.returned = account.collateralReturned;
  exposure.penalties = account.penaltiesPaid;
  return exposure;
}

void mergePositionIntoExposure(OwnerExposure* exposure,
                               const Position& position,
                               const Policy& policy,
                               TimePoint now) {
  if (exposure == nullptr) {
    return;
  }

  const Amount required = requiredCollateralFor(position.debt, policy.minCoverageBps);
  exposure->collateral += position.collateral;
  exposure->debt += position.debt;
  exposure->requiredCollateral += required;
  exposure->surplus += quoteSurplus(position.collateral, required);
  exposure->shortfall += quoteShortfall(position.collateral, required);

  if (position.terminal) {
    exposure->terminalPositions += 1;
  } else if (position.dueAt < now || position.state == PositionState::Matured ||
             position.state == PositionState::Defaulted) {
    exposure->expiredPositions += 1;
  } else {
    exposure->activePositions += 1;
  }
}

}  // namespace granite
