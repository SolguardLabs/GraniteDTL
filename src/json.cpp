#include "json.hpp"

#include "invariant.hpp"
#include "planner.hpp"

#include <sstream>

namespace granite {

namespace {

void writePolicy(JsonWriter& json, const Policy& policy) {
  json.beginObject();
  json.key("min_coverage_bps");
  json.integerValue(policy.minCoverageBps);
  json.comma();
  json.key("target_coverage_bps");
  json.integerValue(policy.targetCoverageBps);
  json.comma();
  json.key("default_penalty_bps");
  json.integerValue(policy.defaultPenaltyBps);
  json.comma();
  json.key("daily_penalty_bps");
  json.integerValue(policy.dailyPenaltyBps);
  json.comma();
  json.key("max_penalty_bps");
  json.integerValue(policy.maxPenaltyBps);
  json.comma();
  json.key("release_fee_bps");
  json.integerValue(policy.releaseFeeBps);
  json.comma();
  json.key("default_ttl");
  json.integerValue(policy.defaultTtl);
  json.comma();
  json.key("grace_period");
  json.integerValue(policy.gracePeriod);
  json.comma();
  json.key("reserve_floor");
  json.amountValue(policy.reserveFloor);
  json.comma();
  json.key("max_single_lock");
  json.amountValue(policy.maxSingleLock);
  json.comma();
  json.key("allow_surplus_release");
  json.boolValue(policy.allowSurplusRelease);
  json.endObject();
}

void writeVault(JsonWriter& json, const Vault& vault) {
  json.beginObject();
  json.key("locked_collateral");
  json.amountValue(vault.lockedCollateral);
  json.comma();
  json.key("reserve_cash");
  json.amountValue(vault.reserveCash);
  json.comma();
  json.key("penalty_reserve");
  json.amountValue(vault.penaltyReserve);
  json.comma();
  json.key("released_surplus");
  json.amountValue(vault.releasedSurplus);
  json.comma();
  json.key("seized_collateral");
  json.amountValue(vault.seizedCollateral);
  json.comma();
  json.key("paid_claims");
  json.amountValue(vault.paidClaims);
  json.comma();
  json.key("system_fees");
  json.amountValue(vault.systemFees);
  json.comma();
  json.key("insolvency");
  json.amountValue(vault.insolvency);
  json.comma();
  json.key("accounting_drift");
  json.amountValue(vault.accountingDrift);
  json.comma();
  json.key("version");
  json.integerValue(vault.version);
  json.endObject();
}

void writeAccount(JsonWriter& json, const Account& account) {
  json.beginObject();
  json.key("id");
  json.stringValue(account.id);
  json.comma();
  json.key("label");
  json.stringValue(account.label);
  json.comma();
  json.key("cash");
  json.amountValue(account.cash);
  json.comma();
  json.key("collateral_posted");
  json.amountValue(account.collateralPosted);
  json.comma();
  json.key("collateral_returned");
  json.amountValue(account.collateralReturned);
  json.comma();
  json.key("surplus_withdrawn");
  json.amountValue(account.surplusWithdrawn);
  json.comma();
  json.key("claims_received");
  json.amountValue(account.claimsReceived);
  json.comma();
  json.key("fees_paid");
  json.amountValue(account.feesPaid);
  json.comma();
  json.key("penalties_paid");
  json.amountValue(account.penaltiesPaid);
  json.comma();
  json.key("external_withdrawals");
  json.amountValue(account.externalWithdrawals);
  json.comma();
  json.key("active");
  json.boolValue(account.active);
  json.endObject();
}

void writeAccounts(JsonWriter& json, const std::map<std::string, Account>& accounts) {
  json.beginObject();
  bool first = true;
  for (const auto& item : accounts) {
    if (!first) {
      json.comma();
    }
    first = false;
    json.key(item.first);
    writeAccount(json, item.second);
  }
  json.endObject();
}

void writePosition(JsonWriter& json, const Position& position) {
  json.beginObject();
  json.key("id");
  json.stringValue(position.id);
  json.comma();
  json.key("owner");
  json.stringValue(position.owner);
  json.comma();
  json.key("counterparty");
  json.stringValue(position.counterparty);
  json.comma();
  json.key("lane");
  json.stringValue(position.lane);
  json.comma();
  json.key("parent");
  json.stringValue(position.parent);
  json.comma();
  json.key("state");
  json.stringValue(toString(position.state));
  json.comma();
  json.key("lock_state");
  json.stringValue(toString(position.lockState));
  json.comma();
  json.key("original_collateral");
  json.amountValue(position.originalCollateral);
  json.comma();
  json.key("collateral");
  json.amountValue(position.collateral);
  json.comma();
  json.key("debt");
  json.amountValue(position.debt);
  json.comma();
  json.key("reserve_claim");
  json.amountValue(position.reserveClaim);
  json.comma();
  json.key("penalty_accrued");
  json.amountValue(position.penaltyAccrued);
  json.comma();
  json.key("surplus_released");
  json.amountValue(position.surplusReleased);
  json.comma();
  json.key("release_fees");
  json.amountValue(position.releaseFees);
  json.comma();
  json.key("claim_paid");
  json.amountValue(position.claimPaid);
  json.comma();
  json.key("shortfall");
  json.amountValue(position.shortfall);
  json.comma();
  json.key("last_quoted_surplus");
  json.amountValue(position.lastQuotedSurplus);
  json.comma();
  json.key("last_required_collateral");
  json.amountValue(position.lastRequiredCollateral);
  json.comma();
  json.key("last_penalty_quote");
  json.amountValue(position.lastPenaltyQuote);
  json.comma();
  json.key("penalty_bps");
  json.integerValue(position.penaltyBps);
  json.comma();
  json.key("cached_coverage_bps");
  json.integerValue(position.cachedCoverageBps);
  json.comma();
  json.key("real_coverage_bps");
  json.integerValue(position.realCoverageBps);
  json.comma();
  json.key("opened_at");
  json.integerValue(position.openedAt);
  json.comma();
  json.key("due_at");
  json.integerValue(position.dueAt);
  json.comma();
  json.key("last_risk_at");
  json.integerValue(position.lastRiskAt);
  json.comma();
  json.key("last_penalty_at");
  json.integerValue(position.lastPenaltyAt);
  json.comma();
  json.key("closed_at");
  json.integerValue(position.closedAt);
  json.comma();
  json.key("unlock_epoch");
  json.integerValue(position.unlockEpoch);
  json.comma();
  json.key("close_attempts");
  json.integerValue(position.closeAttempts);
  json.comma();
  json.key("breach_notified");
  json.boolValue(position.breachNotified);
  json.comma();
  json.key("terminal");
  json.boolValue(position.terminal);
  json.endObject();
}

void writePositions(JsonWriter& json, const std::map<std::string, Position>& positions) {
  json.beginObject();
  bool first = true;
  for (const auto& item : positions) {
    if (!first) {
      json.comma();
    }
    first = false;
    json.key(item.first);
    writePosition(json, item.second);
  }
  json.endObject();
}

void writeRiskView(JsonWriter& json, const CoverageView& view) {
  json.beginObject();
  json.key("position_id");
  json.stringValue(view.positionId);
  json.comma();
  json.key("collateral");
  json.amountValue(view.collateral);
  json.comma();
  json.key("debt");
  json.amountValue(view.debt);
  json.comma();
  json.key("required_collateral");
  json.amountValue(view.requiredCollateral);
  json.comma();
  json.key("surplus");
  json.amountValue(view.surplus);
  json.comma();
  json.key("shortfall");
  json.amountValue(view.shortfall);
  json.comma();
  json.key("coverage_bps");
  json.integerValue(view.coverageBps);
  json.comma();
  json.key("classification");
  json.stringValue(classifyCoverage(view.coverageBps, 15'000));
  json.comma();
  json.key("healthy");
  json.boolValue(view.healthy);
  json.comma();
  json.key("expired");
  json.boolValue(view.expired);
  json.comma();
  json.key("stale");
  json.boolValue(view.stale);
  json.comma();
  json.key("observed_at");
  json.integerValue(view.observedAt);
  json.comma();
  json.key("vault_version");
  json.integerValue(view.vaultVersion);
  json.endObject();
}

void writeRiskViews(JsonWriter& json, const RiskBook& risk) {
  json.beginArray();
  bool first = true;
  for (const CoverageView& view : risk.views()) {
    if (!first) {
      json.comma();
    }
    first = false;
    writeRiskView(json, view);
  }
  json.endArray();
}

void writeEvent(JsonWriter& json, const Event& event) {
  json.beginObject();
  json.key("seq");
  json.integerValue(event.seq);
  json.comma();
  json.key("at");
  json.integerValue(event.at);
  json.comma();
  json.key("kind");
  json.stringValue(toString(event.kind));
  json.comma();
  json.key("account");
  json.stringValue(event.account);
  json.comma();
  json.key("position");
  json.stringValue(event.position);
  json.comma();
  json.key("lane");
  json.stringValue(event.lane);
  json.comma();
  json.key("amount");
  json.amountValue(event.amount);
  json.comma();
  json.key("bps");
  json.integerValue(event.bps);
  json.comma();
  json.key("detail");
  json.stringValue(event.detail);
  json.endObject();
}

void writeEvents(JsonWriter& json, const std::vector<Event>& events) {
  json.beginArray();
  bool first = true;
  for (const Event& event : events) {
    if (!first) {
      json.comma();
    }
    first = false;
    writeEvent(json, event);
  }
  json.endArray();
}

void writeChecks(JsonWriter& json, const CheckSummary& checks) {
  json.beginObject();
  json.key("accounting_ok");
  json.boolValue(checks.accountingOk);
  json.comma();
  json.key("reserve_floor_ok");
  json.boolValue(checks.reserveFloorOk);
  json.comma();
  json.key("no_negative_accounts");
  json.boolValue(checks.noNegativeAccounts);
  json.comma();
  json.key("no_negative_positions");
  json.boolValue(checks.noNegativePositions);
  json.comma();
  json.key("terminal_coverage_ok");
  json.boolValue(checks.terminalCoverageOk);
  json.comma();
  json.key("total_account_cash");
  json.amountValue(checks.totalAccountCash);
  json.comma();
  json.key("total_position_collateral");
  json.amountValue(checks.totalPositionCollateral);
  json.comma();
  json.key("total_system_funds");
  json.amountValue(checks.totalSystemFunds);
  json.comma();
  json.key("computed_funds");
  json.amountValue(checks.computedFunds);
  json.endObject();
}

void writePositionAudit(JsonWriter& json, const PositionAudit& audit) {
  json.beginObject();
  json.key("id");
  json.stringValue(audit.id);
  json.comma();
  json.key("owner");
  json.stringValue(audit.owner);
  json.comma();
  json.key("counterparty");
  json.stringValue(audit.counterparty);
  json.comma();
  json.key("lane");
  json.stringValue(audit.lane);
  json.comma();
  json.key("state");
  json.stringValue(audit.state);
  json.comma();
  json.key("lock_state");
  json.stringValue(audit.lockState);
  json.comma();
  json.key("coverage_class");
  json.stringValue(audit.coverageClass);
  json.comma();
  json.key("collateral");
  json.amountValue(audit.collateral);
  json.comma();
  json.key("debt");
  json.amountValue(audit.debt);
  json.comma();
  json.key("required_collateral");
  json.amountValue(audit.requiredCollateral);
  json.comma();
  json.key("surplus");
  json.amountValue(audit.surplus);
  json.comma();
  json.key("shortfall");
  json.amountValue(audit.shortfall);
  json.comma();
  json.key("penalty_accrued");
  json.amountValue(audit.penaltyAccrued);
  json.comma();
  json.key("surplus_released");
  json.amountValue(audit.surplusReleased);
  json.comma();
  json.key("coverage_bps");
  json.integerValue(audit.coverageBps);
  json.comma();
  json.key("expired");
  json.boolValue(audit.expired);
  json.comma();
  json.key("terminal");
  json.boolValue(audit.terminal);
  json.comma();
  json.key("defaulted");
  json.boolValue(audit.defaulted);
  json.endObject();
}

void writeLaneAudit(JsonWriter& json, const LaneAudit& lane) {
  json.beginObject();
  json.key("lane");
  json.stringValue(lane.lane);
  json.comma();
  json.key("positions");
  json.integerValue(lane.positions);
  json.comma();
  json.key("active");
  json.integerValue(lane.active);
  json.comma();
  json.key("matured");
  json.integerValue(lane.matured);
  json.comma();
  json.key("defaulted");
  json.integerValue(lane.defaulted);
  json.comma();
  json.key("terminal");
  json.integerValue(lane.terminal);
  json.comma();
  json.key("collateral");
  json.amountValue(lane.collateral);
  json.comma();
  json.key("debt");
  json.amountValue(lane.debt);
  json.comma();
  json.key("required_collateral");
  json.amountValue(lane.requiredCollateral);
  json.comma();
  json.key("surplus");
  json.amountValue(lane.surplus);
  json.comma();
  json.key("shortfall");
  json.amountValue(lane.shortfall);
  json.comma();
  json.key("penalties");
  json.amountValue(lane.penalties);
  json.comma();
  json.key("released");
  json.amountValue(lane.released);
  json.endObject();
}

void writeAccountAudit(JsonWriter& json, const AccountAudit& audit) {
  json.beginObject();
  json.key("id");
  json.stringValue(audit.id);
  json.comma();
  json.key("cash");
  json.amountValue(audit.cash);
  json.comma();
  json.key("collateral_posted");
  json.amountValue(audit.collateralPosted);
  json.comma();
  json.key("collateral_returned");
  json.amountValue(audit.collateralReturned);
  json.comma();
  json.key("claims_received");
  json.amountValue(audit.claimsReceived);
  json.comma();
  json.key("penalties_paid");
  json.amountValue(audit.penaltiesPaid);
  json.comma();
  json.key("external_withdrawals");
  json.amountValue(audit.externalWithdrawals);
  json.comma();
  json.key("net_collateral_out");
  json.amountValue(audit.netCollateralOut);
  json.comma();
  json.key("negative");
  json.boolValue(audit.negative);
  json.endObject();
}

void writeAuditSummary(JsonWriter& json, const AuditSummary& audit) {
  json.beginObject();
  json.key("positions");
  json.beginArray();
  bool firstPosition = true;
  for (const PositionAudit& position : audit.positions) {
    if (!firstPosition) {
      json.comma();
    }
    firstPosition = false;
    writePositionAudit(json, position);
  }
  json.endArray();
  json.comma();
  json.key("accounts");
  json.beginArray();
  bool firstAccount = true;
  for (const AccountAudit& account : audit.accounts) {
    if (!firstAccount) {
      json.comma();
    }
    firstAccount = false;
    writeAccountAudit(json, account);
  }
  json.endArray();
  json.comma();
  json.key("lanes");
  json.beginObject();
  bool firstLane = true;
  for (const auto& item : audit.lanes) {
    if (!firstLane) {
      json.comma();
    }
    firstLane = false;
    json.key(item.first);
    writeLaneAudit(json, item.second);
  }
  json.endObject();
  json.comma();
  json.key("warnings");
  json.beginArray();
  bool firstWarning = true;
  for (const std::string& warning : audit.warnings) {
    if (!firstWarning) {
      json.comma();
    }
    firstWarning = false;
    json.stringValue(warning);
  }
  json.endArray();
  json.comma();
  json.key("total_debt");
  json.amountValue(audit.totalDebt);
  json.comma();
  json.key("total_collateral");
  json.amountValue(audit.totalCollateral);
  json.comma();
  json.key("total_required_collateral");
  json.amountValue(audit.totalRequiredCollateral);
  json.comma();
  json.key("total_surplus");
  json.amountValue(audit.totalSurplus);
  json.comma();
  json.key("total_shortfall");
  json.amountValue(audit.totalShortfall);
  json.comma();
  json.key("total_penalties");
  json.amountValue(audit.totalPenalties);
  json.comma();
  json.key("total_released");
  json.amountValue(audit.totalReleased);
  json.comma();
  json.key("total_claims");
  json.amountValue(audit.totalClaims);
  json.comma();
  json.key("aggregate_coverage_bps");
  json.integerValue(audit.aggregateCoverageBps);
  json.comma();
  json.key("open_positions");
  json.integerValue(audit.openPositions);
  json.comma();
  json.key("terminal_positions");
  json.integerValue(audit.terminalPositions);
  json.comma();
  json.key("defaulted_positions");
  json.integerValue(audit.defaultedPositions);
  json.comma();
  json.key("has_warnings");
  json.boolValue(audit.hasWarnings);
  json.endObject();
}

void writeStringIntegerMap(JsonWriter& json, const std::map<std::string, std::int64_t>& map) {
  json.beginObject();
  bool first = true;
  for (const auto& item : map) {
    if (!first) {
      json.comma();
    }
    first = false;
    json.key(item.first);
    json.integerValue(item.second);
  }
  json.endObject();
}

void writeStringAmountMap(JsonWriter& json, const std::map<std::string, Amount>& map) {
  json.beginObject();
  bool first = true;
  for (const auto& item : map) {
    if (!first) {
      json.comma();
    }
    first = false;
    json.key(item.first);
    json.amountValue(item.second);
  }
  json.endObject();
}

void writeJournalStats(JsonWriter& json, const JournalStats& journal) {
  json.beginObject();
  json.key("by_kind");
  writeStringIntegerMap(json, journal.byKind);
  json.comma();
  json.key("by_position");
  writeStringIntegerMap(json, journal.byPosition);
  json.comma();
  json.key("amounts_by_kind");
  writeStringAmountMap(json, journal.amountsByKind);
  json.comma();
  json.key("rejection_details");
  json.beginArray();
  bool first = true;
  for (const std::string& detail : journal.rejectionDetails) {
    if (!first) {
      json.comma();
    }
    first = false;
    json.stringValue(detail);
  }
  json.endArray();
  json.comma();
  json.key("total_events");
  json.integerValue(journal.totalEvents);
  json.comma();
  json.key("rejected_events");
  json.integerValue(journal.rejectedEvents);
  json.comma();
  json.key("risk_refreshes");
  json.integerValue(journal.riskRefreshes);
  json.comma();
  json.key("releases");
  json.integerValue(journal.releases);
  json.comma();
  json.key("penalties");
  json.integerValue(journal.penalties);
  json.comma();
  json.key("released_amount");
  json.amountValue(journal.releasedAmount);
  json.comma();
  json.key("penalty_amount");
  json.amountValue(journal.penaltyAmount);
  json.comma();
  json.key("claim_amount");
  json.amountValue(journal.claimAmount);
  json.comma();
  json.key("withdrawal_amount");
  json.amountValue(journal.withdrawalAmount);
  json.endObject();
}

void writeInvariantCheck(JsonWriter& json, const InvariantCheck& check) {
  json.beginObject();
  json.key("name");
  json.stringValue(check.name);
  json.comma();
  json.key("ok");
  json.boolValue(check.ok);
  json.comma();
  json.key("expected");
  json.amountValue(check.expected);
  json.comma();
  json.key("observed");
  json.amountValue(check.observed);
  json.comma();
  json.key("detail");
  json.stringValue(check.detail);
  json.endObject();
}

void writeInvariantReport(JsonWriter& json, const InvariantReport& report) {
  json.beginObject();
  json.key("ok");
  json.boolValue(report.ok);
  json.comma();
  json.key("checks");
  json.beginArray();
  bool first = true;
  for (const InvariantCheck& check : report.checks) {
    if (!first) {
      json.comma();
    }
    first = false;
    writeInvariantCheck(json, check);
  }
  json.endArray();
  json.endObject();
}

void writeExpiryBucket(JsonWriter& json, const ExpiryBucket& bucket) {
  json.beginObject();
  json.key("epoch");
  json.integerValue(bucket.epoch);
  json.comma();
  json.key("positions");
  json.integerValue(bucket.positions);
  json.comma();
  json.key("collateral");
  json.amountValue(bucket.collateral);
  json.comma();
  json.key("debt");
  json.amountValue(bucket.debt);
  json.comma();
  json.key("required_collateral");
  json.amountValue(bucket.requiredCollateral);
  json.comma();
  json.key("surplus");
  json.amountValue(bucket.surplus);
  json.comma();
  json.key("shortfall");
  json.amountValue(bucket.shortfall);
  json.comma();
  json.key("penalty_quote");
  json.amountValue(bucket.penaltyQuote);
  json.endObject();
}

void writeOwnerExposure(JsonWriter& json, const OwnerExposure& exposure) {
  json.beginObject();
  json.key("account");
  json.stringValue(exposure.account);
  json.comma();
  json.key("active_positions");
  json.integerValue(exposure.activePositions);
  json.comma();
  json.key("expired_positions");
  json.integerValue(exposure.expiredPositions);
  json.comma();
  json.key("terminal_positions");
  json.integerValue(exposure.terminalPositions);
  json.comma();
  json.key("cash");
  json.amountValue(exposure.cash);
  json.comma();
  json.key("collateral");
  json.amountValue(exposure.collateral);
  json.comma();
  json.key("debt");
  json.amountValue(exposure.debt);
  json.comma();
  json.key("required_collateral");
  json.amountValue(exposure.requiredCollateral);
  json.comma();
  json.key("surplus");
  json.amountValue(exposure.surplus);
  json.comma();
  json.key("shortfall");
  json.amountValue(exposure.shortfall);
  json.comma();
  json.key("posted");
  json.amountValue(exposure.posted);
  json.comma();
  json.key("returned");
  json.amountValue(exposure.returned);
  json.comma();
  json.key("penalties");
  json.amountValue(exposure.penalties);
  json.endObject();
}

void writePortfolioReport(JsonWriter& json, const PortfolioReport& portfolio) {
  json.beginObject();
  json.key("buckets");
  json.beginArray();
  bool firstBucket = true;
  for (const ExpiryBucket& bucket : portfolio.buckets) {
    if (!firstBucket) {
      json.comma();
    }
    firstBucket = false;
    writeExpiryBucket(json, bucket);
  }
  json.endArray();
  json.comma();
  json.key("owners");
  json.beginArray();
  bool firstOwner = true;
  for (const OwnerExposure& owner : portfolio.owners) {
    if (!firstOwner) {
      json.comma();
    }
    firstOwner = false;
    writeOwnerExposure(json, owner);
  }
  json.endArray();
  json.comma();
  json.key("next_expiry");
  json.integerValue(portfolio.nextExpiry);
  json.comma();
  json.key("unlockable_now");
  json.amountValue(portfolio.unlockableNow);
  json.comma();
  json.key("locked_until_due");
  json.amountValue(portfolio.lockedUntilDue);
  json.comma();
  json.key("projected_penalty");
  json.amountValue(portfolio.projectedPenalty);
  json.comma();
  json.key("projected_shortfall");
  json.amountValue(portfolio.projectedShortfall);
  json.comma();
  json.key("max_bucket_shortfall");
  json.amountValue(portfolio.maxBucketShortfall);
  json.comma();
  json.key("riskiest_owner");
  json.stringValue(portfolio.riskiestOwner);
  json.endObject();
}

void writePlannedAction(JsonWriter& json, const PlannedAction& action) {
  json.beginObject();
  json.key("position");
  json.stringValue(action.position);
  json.comma();
  json.key("account");
  json.stringValue(action.account);
  json.comma();
  json.key("action");
  json.stringValue(action.action);
  json.comma();
  json.key("reason");
  json.stringValue(action.reason);
  json.comma();
  json.key("amount");
  json.amountValue(action.amount);
  json.comma();
  json.key("due_at");
  json.integerValue(action.dueAt);
  json.comma();
  json.key("priority");
  json.integerValue(action.priority);
  json.endObject();
}

void writePlanReport(JsonWriter& json, const PlanReport& plan) {
  json.beginObject();
  json.key("actions");
  json.beginArray();
  bool first = true;
  for (const PlannedAction& action : plan.actions) {
    if (!first) {
      json.comma();
    }
    first = false;
    writePlannedAction(json, action);
  }
  json.endArray();
  json.comma();
  json.key("immediate");
  json.integerValue(plan.immediate);
  json.comma();
  json.key("deferred");
  json.integerValue(plan.deferred);
  json.comma();
  json.key("highest_priority");
  json.integerValue(plan.highestPriority);
  json.comma();
  json.key("has_liquidations");
  json.boolValue(plan.hasLiquidations);
  json.endObject();
}

}  // namespace

JsonWriter::JsonWriter(std::ostream& out) : out_(out) {}

void JsonWriter::beginObject() {
  out_ << "{";
  depth_++;
}

void JsonWriter::endObject() {
  depth_--;
  out_ << "}";
}

void JsonWriter::beginArray() {
  out_ << "[";
  depth_++;
}

void JsonWriter::endArray() {
  depth_--;
  out_ << "]";
}

void JsonWriter::key(const std::string& name) {
  stringValue(name);
  out_ << ":";
}

void JsonWriter::stringValue(const std::string& value) {
  out_ << "\"" << jsonEscape(value) << "\"";
}

void JsonWriter::amountValue(Amount value) {
  out_ << value;
}

void JsonWriter::integerValue(std::int64_t value) {
  out_ << value;
}

void JsonWriter::boolValue(bool value) {
  out_ << (value ? "true" : "false");
}

void JsonWriter::nullValue() {
  out_ << "null";
}

void JsonWriter::comma() {
  out_ << ",";
}

void JsonWriter::newline() {
  out_ << "\n";
}

void JsonWriter::indent() {
  for (int i = 0; i < depth_; i++) {
    out_ << "  ";
  }
}

std::string jsonEscape(const std::string& value) {
  std::ostringstream out;
  for (const char ch : value) {
    switch (ch) {
      case '"':
        out << "\\\"";
        break;
      case '\\':
        out << "\\\\";
        break;
      case '\b':
        out << "\\b";
        break;
      case '\f':
        out << "\\f";
        break;
      case '\n':
        out << "\\n";
        break;
      case '\r':
        out << "\\r";
        break;
      case '\t':
        out << "\\t";
        break;
      default:
        if (static_cast<unsigned char>(ch) < 0x20) {
          out << "\\u00";
          const char* hex = "0123456789abcdef";
          out << hex[(ch >> 4) & 0x0f];
          out << hex[ch & 0x0f];
        } else {
          out << ch;
        }
        break;
    }
  }
  return out.str();
}

void writeReport(std::ostream& out, const Engine& engine) {
  JsonWriter json(out);
  const AuditSummary audit = buildAudit(engine);
  const JournalStats journal = analyzeJournal(engine.events());
  const InvariantReport invariants = evaluateInvariants(engine, audit, journal);
  const PortfolioReport portfolio = buildPortfolio(engine);
  const PlanReport plan = buildPlan(engine, audit, portfolio);

  json.beginObject();
  json.key("scenario");
  json.stringValue(engine.scenarioName());
  json.comma();
  json.key("clock");
  json.integerValue(engine.now());
  json.comma();
  json.key("policy");
  writePolicy(json, engine.policy());
  json.comma();
  json.key("vault");
  writeVault(json, engine.vault());
  json.comma();
  json.key("accounts");
  writeAccounts(json, engine.accounts());
  json.comma();
  json.key("positions");
  writePositions(json, engine.positions());
  json.comma();
  json.key("risk_views");
  writeRiskViews(json, engine.riskBook());
  json.comma();
  json.key("events");
  writeEvents(json, engine.events());
  json.comma();
  json.key("checks");
  writeChecks(json, engine.checks());
  json.comma();
  json.key("audit");
  writeAuditSummary(json, audit);
  json.comma();
  json.key("journal");
  writeJournalStats(json, journal);
  json.comma();
  json.key("invariants");
  writeInvariantReport(json, invariants);
  json.comma();
  json.key("portfolio");
  writePortfolioReport(json, portfolio);
  json.comma();
  json.key("plan");
  writePlanReport(json, plan);
  json.comma();
  json.key("last_error");
  if (engine.lastError().empty()) {
    json.nullValue();
  } else {
    json.stringValue(engine.lastError());
  }
  json.comma();
  json.key("external_inflows");
  json.amountValue(engine.externalInflows());
  json.comma();
  json.key("external_withdrawals");
  json.amountValue(engine.externalWithdrawals());
  json.endObject();
  json.newline();
}

void writeScenarioList(std::ostream& out, const std::vector<std::string>& names) {
  JsonWriter json(out);
  json.beginObject();
  json.key("scenarios");
  json.beginArray();
  bool first = true;
  for (const std::string& name : names) {
    if (!first) {
      json.comma();
    }
    first = false;
    json.stringValue(name);
  }
  json.endArray();
  json.endObject();
  json.newline();
}

}  // namespace granite
