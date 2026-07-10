#include "audit.hpp"

#include <algorithm>

namespace granite {

namespace {

void addAmount(Amount* target, Amount value) {
  if (target == nullptr) {
    return;
  }

  *target += value;
}

bool isDefaultState(const std::string& state) {
  return state == "defaulted";
}

bool isTerminalState(const std::string& state) {
  return state == "closed" || state == "liquidated";
}

std::string warningForLane(const LaneAudit& lane) {
  if (lane.positions == 0) {
    return "";
  }

  if (lane.shortfall > 0) {
    return "lane " + lane.lane + " carries coverage shortfall " +
           std::to_string(lane.shortfall);
  }

  if (lane.defaulted > 0 && lane.terminal == 0) {
    return "lane " + lane.lane + " has unresolved defaulted positions";
  }

  if (lane.released > lane.surplus + lane.penalties && lane.shortfall > 0) {
    return "lane " + lane.lane + " released collateral while shortfall is present";
  }

  return "";
}

}  // namespace

AuditSummary buildAudit(const Engine& engine) {
  AuditSummary summary;
  const Policy& policy = engine.policy();
  const Vault& vault = engine.vault();

  for (const auto& item : engine.positions()) {
    PositionAudit position = auditPosition(item.second, policy, engine.now(), vault.version);
    summary.positions.push_back(position);

    addAmount(&summary.totalDebt, position.debt);
    addAmount(&summary.totalCollateral, position.collateral);
    addAmount(&summary.totalRequiredCollateral, position.requiredCollateral);
    addAmount(&summary.totalSurplus, position.surplus);
    addAmount(&summary.totalShortfall, position.shortfall);
    addAmount(&summary.totalPenalties, position.penaltyAccrued);
    addAmount(&summary.totalReleased, position.surplusReleased);

    if (position.terminal) {
      summary.terminalPositions += 1;
    } else {
      summary.openPositions += 1;
    }

    if (position.defaulted) {
      summary.defaultedPositions += 1;
    }

    LaneAudit& lane = summary.lanes[position.lane];
    if (lane.lane.empty()) {
      lane.lane = position.lane;
    }
    mergeLaneAudit(&lane, position);
  }

  for (const auto& item : engine.accounts()) {
    AccountAudit account = auditAccount(item.second);
    summary.accounts.push_back(account);
    addAmount(&summary.totalClaims, account.claimsReceived);
  }

  summary.aggregateCoverageBps =
      quoteCoverageBps(summary.totalCollateral, summary.totalDebt);
  summary.warnings = buildAuditWarnings(summary, engine);
  summary.hasWarnings = !summary.warnings.empty();
  return summary;
}

PositionAudit auditPosition(const Position& position,
                            const Policy& policy,
                            TimePoint now,
                            std::int64_t vaultVersion) {
  RiskBook risk;
  CoverageView view = risk.observe(position, policy, now, vaultVersion);

  PositionAudit audit;
  audit.id = position.id;
  audit.owner = position.owner;
  audit.counterparty = position.counterparty;
  audit.lane = position.lane;
  audit.state = toString(position.state);
  audit.lockState = toString(position.lockState);
  audit.coverageClass = classifyCoverage(view.coverageBps, policy.minCoverageBps);
  audit.collateral = position.collateral;
  audit.debt = position.debt;
  audit.requiredCollateral = view.requiredCollateral;
  audit.surplus = view.surplus;
  audit.shortfall = view.shortfall;
  audit.penaltyAccrued = position.penaltyAccrued;
  audit.surplusReleased = position.surplusReleased;
  audit.coverageBps = view.coverageBps;
  audit.expired = view.expired;
  audit.terminal = position.terminal;
  audit.defaulted = position.state == PositionState::Defaulted;
  return audit;
}

AccountAudit auditAccount(const Account& account) {
  AccountAudit audit;
  audit.id = account.id;
  audit.cash = account.cash;
  audit.collateralPosted = account.collateralPosted;
  audit.collateralReturned = account.collateralReturned;
  audit.claimsReceived = account.claimsReceived;
  audit.penaltiesPaid = account.penaltiesPaid;
  audit.externalWithdrawals = account.externalWithdrawals;
  audit.netCollateralOut = account.collateralPosted - account.collateralReturned;
  audit.negative = account.cash < 0;
  return audit;
}

void mergeLaneAudit(LaneAudit* lane, const PositionAudit& position) {
  if (lane == nullptr) {
    return;
  }

  lane->positions += 1;
  lane->collateral += position.collateral;
  lane->debt += position.debt;
  lane->requiredCollateral += position.requiredCollateral;
  lane->surplus += position.surplus;
  lane->shortfall += position.shortfall;
  lane->penalties += position.penaltyAccrued;
  lane->released += position.surplusReleased;

  if (position.state == "active") {
    lane->active += 1;
  }

  if (position.state == "matured") {
    lane->matured += 1;
  }

  if (isDefaultState(position.state)) {
    lane->defaulted += 1;
  }

  if (isTerminalState(position.state)) {
    lane->terminal += 1;
  }
}

std::vector<std::string> buildAuditWarnings(const AuditSummary& summary,
                                            const Engine& engine) {
  std::vector<std::string> warnings;

  if (summary.totalShortfall > 0) {
    warnings.push_back("aggregate coverage shortfall " +
                       std::to_string(summary.totalShortfall));
  }

  if (summary.defaultedPositions > 0) {
    warnings.push_back("defaulted positions present " +
                       std::to_string(summary.defaultedPositions));
  }

  if (engine.vault().insolvency > 0) {
    warnings.push_back("vault insolvency marker " +
                       std::to_string(engine.vault().insolvency));
  }

  if (engine.vault().reserveCash < engine.policy().reserveFloor) {
    warnings.push_back("reserve floor below policy");
  }

  for (const auto& item : summary.lanes) {
    const std::string laneWarning = warningForLane(item.second);
    if (!laneWarning.empty()) {
      warnings.push_back(laneWarning);
    }
  }

  for (const AccountAudit& account : summary.accounts) {
    if (account.negative) {
      warnings.push_back("account " + account.id + " is negative");
    }
  }

  std::sort(warnings.begin(), warnings.end());
  warnings.erase(std::unique(warnings.begin(), warnings.end()), warnings.end());
  return warnings;
}

}  // namespace granite
