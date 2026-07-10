#pragma once

#include "amount.hpp"

#include <map>
#include <string>
#include <vector>

namespace granite {

enum class PositionState {
  Draft,
  Active,
  Matured,
  Defaulted,
  Closed,
  Liquidated,
};

enum class LockState {
  None,
  Timelocked,
  Expired,
  Released,
  Seized,
};

enum class EventKind {
  AccountAdded,
  ReserveAdded,
  PositionOpened,
  ClockAdvanced,
  LockExpired,
  RiskRefreshed,
  PenaltyAccrued,
  SurplusReleased,
  PositionCompleted,
  PositionDefaulted,
  PositionLiquidated,
  ClaimPaid,
  Withdrawal,
  ScriptNote,
  Rejected,
};

enum class SettlementMode {
  Manual,
  Maintenance,
  Scripted,
};

struct Policy {
  BasisPoints minCoverageBps = 15'000;
  BasisPoints targetCoverageBps = 17'000;
  BasisPoints defaultPenaltyBps = 600;
  BasisPoints dailyPenaltyBps = 125;
  BasisPoints maxPenaltyBps = 2'500;
  BasisPoints releaseFeeBps = 25;
  TimePoint defaultTtl = 10;
  TimePoint gracePeriod = 1;
  Amount reserveFloor = 500'000;
  Amount maxSingleLock = 10'000'000;
  bool allowSurplusRelease = true;
};

struct Account {
  std::string id;
  std::string label;
  Amount cash = 0;
  Amount collateralPosted = 0;
  Amount collateralReturned = 0;
  Amount surplusWithdrawn = 0;
  Amount claimsReceived = 0;
  Amount feesPaid = 0;
  Amount penaltiesPaid = 0;
  Amount externalWithdrawals = 0;
  bool active = true;
};

struct Vault {
  Amount lockedCollateral = 0;
  Amount reserveCash = 0;
  Amount penaltyReserve = 0;
  Amount releasedSurplus = 0;
  Amount seizedCollateral = 0;
  Amount paidClaims = 0;
  Amount systemFees = 0;
  Amount insolvency = 0;
  Amount accountingDrift = 0;
  std::int64_t version = 0;
};

struct Position {
  std::string id;
  std::string owner;
  std::string counterparty;
  std::string lane;
  std::string parent;
  PositionState state = PositionState::Draft;
  LockState lockState = LockState::None;
  Amount originalCollateral = 0;
  Amount collateral = 0;
  Amount debt = 0;
  Amount reserveClaim = 0;
  Amount penaltyAccrued = 0;
  Amount surplusReleased = 0;
  Amount releaseFees = 0;
  Amount claimPaid = 0;
  Amount shortfall = 0;
  Amount lastQuotedSurplus = 0;
  Amount lastRequiredCollateral = 0;
  Amount lastPenaltyQuote = 0;
  BasisPoints penaltyBps = 0;
  BasisPoints cachedCoverageBps = 0;
  BasisPoints realCoverageBps = 0;
  TimePoint openedAt = 0;
  TimePoint dueAt = 0;
  TimePoint lastRiskAt = 0;
  TimePoint lastPenaltyAt = 0;
  TimePoint closedAt = 0;
  TimePoint unlockEpoch = 0;
  std::int64_t closeAttempts = 0;
  bool breachNotified = false;
  bool terminal = false;
};

struct CoverageView {
  std::string positionId;
  Amount collateral = 0;
  Amount debt = 0;
  Amount requiredCollateral = 0;
  Amount surplus = 0;
  Amount shortfall = 0;
  BasisPoints coverageBps = 0;
  bool healthy = false;
  bool expired = false;
  bool stale = false;
  TimePoint observedAt = 0;
  std::int64_t vaultVersion = 0;
};

struct Event {
  std::int64_t seq = 0;
  TimePoint at = 0;
  EventKind kind = EventKind::ScriptNote;
  std::string account;
  std::string position;
  std::string lane;
  Amount amount = 0;
  BasisPoints bps = 0;
  std::string detail;
};

struct CheckSummary {
  bool accountingOk = true;
  bool reserveFloorOk = true;
  bool noNegativeAccounts = true;
  bool noNegativePositions = true;
  bool terminalCoverageOk = true;
  Amount totalAccountCash = 0;
  Amount totalPositionCollateral = 0;
  Amount totalSystemFunds = 0;
  Amount computedFunds = 0;
};

std::string toString(PositionState state);

std::string toString(LockState state);

std::string toString(EventKind kind);

std::string toString(SettlementMode mode);

PositionState parsePositionState(const std::string& text);

bool isTerminal(PositionState state);

bool isOpen(PositionState state);

}  // namespace granite
