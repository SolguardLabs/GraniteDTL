#pragma once

#include "audit.hpp"
#include "portfolio.hpp"

#include <string>
#include <vector>

namespace granite {

struct PlannedAction {
  std::string position;
  std::string account;
  std::string action;
  std::string reason;
  Amount amount = 0;
  TimePoint dueAt = 0;
  int priority = 0;
};

struct PlanReport {
  std::vector<PlannedAction> actions;
  std::int64_t immediate = 0;
  std::int64_t deferred = 0;
  int highestPriority = 0;
  bool hasLiquidations = false;
};

PlanReport buildPlan(const Engine& engine,
                     const AuditSummary& audit,
                     const PortfolioReport& portfolio);

PlannedAction planForPosition(const PositionAudit& audit,
                              TimePoint now,
                              BasisPoints minCoverageBps);

PlannedAction planForOwner(const OwnerExposure& exposure);

bool comparePlannedAction(const PlannedAction& left, const PlannedAction& right);

}  // namespace granite
