#pragma once

#include "engine.hpp"

#include <map>
#include <string>
#include <vector>

namespace granite {

struct PositionAudit {
  std::string id;
  std::string owner;
  std::string counterparty;
  std::string lane;
  std::string state;
  std::string lockState;
  std::string coverageClass;
  Amount collateral = 0;
  Amount debt = 0;
  Amount requiredCollateral = 0;
  Amount surplus = 0;
  Amount shortfall = 0;
  Amount penaltyAccrued = 0;
  Amount surplusReleased = 0;
  BasisPoints coverageBps = 0;
  bool expired = false;
  bool terminal = false;
  bool defaulted = false;
};

struct LaneAudit {
  std::string lane;
  std::int64_t positions = 0;
  std::int64_t active = 0;
  std::int64_t matured = 0;
  std::int64_t defaulted = 0;
  std::int64_t terminal = 0;
  Amount collateral = 0;
  Amount debt = 0;
  Amount requiredCollateral = 0;
  Amount surplus = 0;
  Amount shortfall = 0;
  Amount penalties = 0;
  Amount released = 0;
};

struct AccountAudit {
  std::string id;
  Amount cash = 0;
  Amount collateralPosted = 0;
  Amount collateralReturned = 0;
  Amount claimsReceived = 0;
  Amount penaltiesPaid = 0;
  Amount externalWithdrawals = 0;
  Amount netCollateralOut = 0;
  bool negative = false;
};

struct AuditSummary {
  std::vector<PositionAudit> positions;
  std::vector<AccountAudit> accounts;
  std::map<std::string, LaneAudit> lanes;
  std::vector<std::string> warnings;
  Amount totalDebt = 0;
  Amount totalCollateral = 0;
  Amount totalRequiredCollateral = 0;
  Amount totalSurplus = 0;
  Amount totalShortfall = 0;
  Amount totalPenalties = 0;
  Amount totalReleased = 0;
  Amount totalClaims = 0;
  BasisPoints aggregateCoverageBps = 0;
  std::int64_t openPositions = 0;
  std::int64_t terminalPositions = 0;
  std::int64_t defaultedPositions = 0;
  bool hasWarnings = false;
};

AuditSummary buildAudit(const Engine& engine);

PositionAudit auditPosition(const Position& position,
                            const Policy& policy,
                            TimePoint now,
                            std::int64_t vaultVersion);

AccountAudit auditAccount(const Account& account);

void mergeLaneAudit(LaneAudit* lane, const PositionAudit& position);

std::vector<std::string> buildAuditWarnings(const AuditSummary& summary,
                                            const Engine& engine);

}  // namespace granite
