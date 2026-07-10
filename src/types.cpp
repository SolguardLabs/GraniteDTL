#include "types.hpp"

#include <stdexcept>

namespace granite {

std::string toString(PositionState state) {
  switch (state) {
    case PositionState::Draft:
      return "draft";
    case PositionState::Active:
      return "active";
    case PositionState::Matured:
      return "matured";
    case PositionState::Defaulted:
      return "defaulted";
    case PositionState::Closed:
      return "closed";
    case PositionState::Liquidated:
      return "liquidated";
  }

  return "unknown";
}

std::string toString(LockState state) {
  switch (state) {
    case LockState::None:
      return "none";
    case LockState::Timelocked:
      return "timelocked";
    case LockState::Expired:
      return "expired";
    case LockState::Released:
      return "released";
    case LockState::Seized:
      return "seized";
  }

  return "unknown";
}

std::string toString(EventKind kind) {
  switch (kind) {
    case EventKind::AccountAdded:
      return "account_added";
    case EventKind::ReserveAdded:
      return "reserve_added";
    case EventKind::PositionOpened:
      return "position_opened";
    case EventKind::ClockAdvanced:
      return "clock_advanced";
    case EventKind::LockExpired:
      return "lock_expired";
    case EventKind::RiskRefreshed:
      return "risk_refreshed";
    case EventKind::PenaltyAccrued:
      return "penalty_accrued";
    case EventKind::SurplusReleased:
      return "surplus_released";
    case EventKind::PositionCompleted:
      return "position_completed";
    case EventKind::PositionDefaulted:
      return "position_defaulted";
    case EventKind::PositionLiquidated:
      return "position_liquidated";
    case EventKind::ClaimPaid:
      return "claim_paid";
    case EventKind::Withdrawal:
      return "withdrawal";
    case EventKind::ScriptNote:
      return "script_note";
    case EventKind::Rejected:
      return "rejected";
  }

  return "unknown";
}

std::string toString(SettlementMode mode) {
  switch (mode) {
    case SettlementMode::Manual:
      return "manual";
    case SettlementMode::Maintenance:
      return "maintenance";
    case SettlementMode::Scripted:
      return "scripted";
  }

  return "unknown";
}

PositionState parsePositionState(const std::string& text) {
  const std::string lowered = lowerCopy(text);

  if (lowered == "draft") {
    return PositionState::Draft;
  }

  if (lowered == "active") {
    return PositionState::Active;
  }

  if (lowered == "matured") {
    return PositionState::Matured;
  }

  if (lowered == "defaulted") {
    return PositionState::Defaulted;
  }

  if (lowered == "closed") {
    return PositionState::Closed;
  }

  if (lowered == "liquidated") {
    return PositionState::Liquidated;
  }

  throw std::invalid_argument("unknown position state: " + text);
}

bool isTerminal(PositionState state) {
  return state == PositionState::Closed || state == PositionState::Liquidated;
}

bool isOpen(PositionState state) {
  return state == PositionState::Active || state == PositionState::Matured ||
         state == PositionState::Defaulted;
}

}  // namespace granite
