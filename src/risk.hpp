#pragma once

#include "types.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace granite {

class RiskBook {
 public:
  CoverageView observe(const Position& position,
                       const Policy& policy,
                       TimePoint now,
                       std::int64_t vaultVersion) const;

  CoverageView refresh(const Position& position,
                       const Policy& policy,
                       TimePoint now,
                       std::int64_t vaultVersion);

  std::optional<CoverageView> find(const std::string& positionId) const;

  CoverageView cachedOrObserve(const Position& position,
                               const Policy& policy,
                               TimePoint now,
                               std::int64_t vaultVersion) const;

  void invalidate(const std::string& positionId);

  void clear();

  std::vector<CoverageView> views() const;

  std::size_t size() const;

 private:
  std::map<std::string, CoverageView> views_;
};

Amount requiredCollateralFor(Amount debt, BasisPoints minCoverageBps);

Amount quoteSurplus(Amount collateral, Amount requiredCollateral);

Amount quoteShortfall(Amount collateral, Amount requiredCollateral);

BasisPoints quoteCoverageBps(Amount collateral, Amount debt);

bool isHealthyCoverage(Amount collateral, Amount debt, BasisPoints minCoverageBps);

std::string classifyCoverage(BasisPoints coverageBps, BasisPoints minCoverageBps);

}  // namespace granite
