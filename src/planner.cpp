#include "planner.hpp"

#include <algorithm>

namespace granite {

namespace {

PlannedAction makeAction(const std::string& position,
                         const std::string& account,
                         const std::string& action,
                         const std::string& reason,
                         Amount amount,
                         TimePoint dueAt,
                         int priority) {
  PlannedAction planned;
  planned.position = position;
  planned.account = account;
  planned.action = action;
  planned.reason = reason;
  planned.amount = amount;
  planned.dueAt = dueAt;
  planned.priority = priority;
  return planned;
}

}  // namespace

PlanReport buildPlan(const Engine& engine,
                     const AuditSummary& audit,
                     const PortfolioReport& portfolio) {
  PlanReport report;

  for (const PositionAudit& position : audit.positions) {
    PlannedAction action =
        planForPosition(position, engine.now(), engine.policy().minCoverageBps);
    if (!action.action.empty()) {
      report.actions.push_back(action);
    }
  }

  for (const OwnerExposure& owner : portfolio.owners) {
    PlannedAction action = planForOwner(owner);
    if (!action.action.empty()) {
      report.actions.push_back(action);
    }
  }

  std::sort(report.actions.begin(), report.actions.end(), comparePlannedAction);

  for (const PlannedAction& action : report.actions) {
    if (action.priority > report.highestPriority) {
      report.highestPriority = action.priority;
    }

    if (action.priority >= 80) {
      report.immediate += 1;
    } else {
      report.deferred += 1;
    }

    if (action.action == "liquidate") {
      report.hasLiquidations = true;
    }
  }

  return report;
}

PlannedAction planForPosition(const PositionAudit& audit,
                              TimePoint now,
                              BasisPoints minCoverageBps) {
  if (audit.terminal) {
    return makeAction(audit.id, audit.owner, "", "terminal", 0, 0, 0);
  }

  if (audit.shortfall > 0) {
    return makeAction(audit.id, audit.owner, "liquidate", "coverage shortfall",
                      audit.shortfall, now, 100);
  }

  if (audit.defaulted) {
    return makeAction(audit.id, audit.owner, "settle", "default unresolved",
                      audit.requiredCollateral, now, 95);
  }

  if (audit.expired) {
    return makeAction(audit.id, audit.owner, "settle", "expired lock",
                      audit.surplus, now, 85);
  }

  if (audit.coverageBps >= minCoverageBps + 2'000 && audit.surplus > 0) {
    return makeAction(audit.id, audit.owner, "release", "surplus above target",
                      audit.surplus, now, 40);
  }

  return makeAction(audit.id, audit.owner, "refresh", "monitor coverage",
                    audit.collateral, now, 10);
}

PlannedAction planForOwner(const OwnerExposure& exposure) {
  if (exposure.shortfall > 0) {
    return makeAction("", exposure.account, "owner-margin-call", "owner aggregate shortfall",
                      exposure.shortfall, 0, 90);
  }

  if (exposure.expiredPositions > 1) {
    return makeAction("", exposure.account, "owner-review", "multiple expired locks",
                      exposure.collateral, 0, 70);
  }

  if (exposure.surplus > 0 && exposure.activePositions > 0) {
    return makeAction("", exposure.account, "owner-release-review", "aggregate surplus",
                      exposure.surplus, 0, 25);
  }

  return makeAction("", exposure.account, "", "no owner action", 0, 0, 0);
}

bool comparePlannedAction(const PlannedAction& left, const PlannedAction& right) {
  if (left.priority != right.priority) {
    return left.priority > right.priority;
  }

  if (left.position != right.position) {
    return left.position < right.position;
  }

  return left.account < right.account;
}

}  // namespace granite
