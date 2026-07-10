#include "risk.hpp"

#include <algorithm>

namespace granite {

namespace {

Amount ceilMulDiv(Amount value, Amount multiplier, Amount divisor) {
  if (divisor <= 0 || value <= 0 || multiplier <= 0) {
    return 0;
  }

  const MathResult product = AmountMath::mul(value, multiplier);
  if (!product.ok) {
    return kMaxAmount;
  }

  const Amount quotient = product.value / divisor;
  const Amount remainder = product.value % divisor;
  return remainder == 0 ? quotient : quotient + 1;
}

}  // namespace

Amount requiredCollateralFor(Amount debt, BasisPoints minCoverageBps) {
  if (debt <= 0) {
    return 0;
  }

  if (minCoverageBps <= 0) {
    return debt;
  }

  return ceilMulDiv(debt, minCoverageBps, kOneHundredPercentBps);
}

Amount quoteSurplus(Amount collateral, Amount requiredCollateral) {
  if (collateral <= requiredCollateral) {
    return 0;
  }

  return collateral - requiredCollateral;
}

Amount quoteShortfall(Amount collateral, Amount requiredCollateral) {
  if (collateral >= requiredCollateral) {
    return 0;
  }

  return requiredCollateral - collateral;
}

BasisPoints quoteCoverageBps(Amount collateral, Amount debt) {
  const RatioResult ratio = AmountMath::ratioBps(collateral, debt);
  if (!ratio.ok) {
    return 0;
  }

  return ratio.value;
}

bool isHealthyCoverage(Amount collateral, Amount debt, BasisPoints minCoverageBps) {
  if (debt <= 0) {
    return true;
  }

  return quoteCoverageBps(collateral, debt) >= minCoverageBps;
}

std::string classifyCoverage(BasisPoints coverageBps, BasisPoints minCoverageBps) {
  if (coverageBps == 0) {
    return "empty";
  }

  if (coverageBps >= minCoverageBps + 2'500) {
    return "surplus";
  }

  if (coverageBps >= minCoverageBps) {
    return "healthy";
  }

  if (coverageBps + 1'000 >= minCoverageBps) {
    return "thin";
  }

  return "breach";
}

CoverageView RiskBook::observe(const Position& position,
                               const Policy& policy,
                               TimePoint now,
                               std::int64_t vaultVersion) const {
  CoverageView view;
  view.positionId = position.id;
  view.collateral = position.collateral;
  view.debt = position.debt;
  view.requiredCollateral = requiredCollateralFor(position.debt, policy.minCoverageBps);
  view.surplus = quoteSurplus(position.collateral, view.requiredCollateral);
  view.shortfall = quoteShortfall(position.collateral, view.requiredCollateral);
  view.coverageBps = quoteCoverageBps(position.collateral, position.debt);
  view.healthy = view.shortfall == 0;
  view.expired = now > position.dueAt;
  view.stale = false;
  view.observedAt = now;
  view.vaultVersion = vaultVersion;
  return view;
}

CoverageView RiskBook::refresh(const Position& position,
                               const Policy& policy,
                               TimePoint now,
                               std::int64_t vaultVersion) {
  CoverageView view = observe(position, policy, now, vaultVersion);
  views_[position.id] = view;
  return view;
}

std::optional<CoverageView> RiskBook::find(const std::string& positionId) const {
  const auto found = views_.find(positionId);
  if (found == views_.end()) {
    return std::nullopt;
  }

  return found->second;
}

CoverageView RiskBook::cachedOrObserve(const Position& position,
                                       const Policy& policy,
                                       TimePoint now,
                                       std::int64_t vaultVersion) const {
  const auto cached = find(position.id);
  if (cached.has_value()) {
    CoverageView copy = cached.value();
    copy.stale = copy.observedAt != now || copy.vaultVersion != vaultVersion ||
                 copy.collateral != position.collateral || copy.debt != position.debt;
    return copy;
  }

  return observe(position, policy, now, vaultVersion);
}

void RiskBook::invalidate(const std::string& positionId) {
  views_.erase(positionId);
}

void RiskBook::clear() {
  views_.clear();
}

std::vector<CoverageView> RiskBook::views() const {
  std::vector<CoverageView> result;
  result.reserve(views_.size());

  for (const auto& item : views_) {
    result.push_back(item.second);
  }

  return result;
}

std::size_t RiskBook::size() const {
  return views_.size();
}

}  // namespace granite
