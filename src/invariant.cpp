#include "invariant.hpp"

namespace granite {

namespace {

InvariantCheck makeCheck(const std::string& name,
                         bool ok,
                         Amount expected,
                         Amount observed,
                         const std::string& detail) {
  InvariantCheck check;
  check.name = name;
  check.ok = ok;
  check.expected = expected;
  check.observed = observed;
  check.detail = detail;
  return check;
}

}  // namespace

InvariantReport evaluateInvariants(const Engine& engine,
                                   const AuditSummary& audit,
                                   const JournalStats& journal) {
  InvariantReport report;
  report.checks.push_back(checkAccounting(engine));
  report.checks.push_back(checkReserveFloor(engine));
  report.checks.push_back(checkNoNegativeBalances(engine));
  report.checks.push_back(checkOpenCoverage(engine, audit));
  report.checks.push_back(checkJournalConsistency(engine, journal));

  for (const InvariantCheck& check : report.checks) {
    if (!check.ok) {
      report.ok = false;
      break;
    }
  }

  return report;
}

InvariantCheck checkAccounting(const Engine& engine) {
  const CheckSummary checks = engine.checks();
  return makeCheck("accounting",
                   checks.accountingOk,
                   checks.computedFunds,
                   checks.totalSystemFunds,
                   checks.accountingOk ? "funds conserved" : "funds drift");
}

InvariantCheck checkReserveFloor(const Engine& engine) {
  const bool ok = engine.vault().reserveCash >= engine.policy().reserveFloor;
  return makeCheck("reserve_floor",
                   ok,
                   engine.policy().reserveFloor,
                   engine.vault().reserveCash,
                   ok ? "reserve above floor" : "reserve below floor");
}

InvariantCheck checkNoNegativeBalances(const Engine& engine) {
  Amount negatives = 0;

  for (const auto& item : engine.accounts()) {
    if (item.second.cash < 0) {
      negatives += 1;
    }
  }

  for (const auto& item : engine.positions()) {
    if (item.second.collateral < 0) {
      negatives += 1;
    }
  }

  return makeCheck("non_negative_balances",
                   negatives == 0,
                   0,
                   negatives,
                   negatives == 0 ? "all balances non-negative" : "negative balances present");
}

InvariantCheck checkOpenCoverage(const Engine& engine, const AuditSummary& audit) {
  (void)engine;
  Amount shortfall = 0;

  for (const PositionAudit& position : audit.positions) {
    if (!position.terminal && position.shortfall > 0) {
      shortfall += position.shortfall;
    }
  }

  return makeCheck("open_coverage",
                   shortfall == 0,
                   0,
                   shortfall,
                   shortfall == 0 ? "open positions covered" : "open coverage shortfall");
}

InvariantCheck checkJournalConsistency(const Engine& engine, const JournalStats& journal) {
  const Amount releasedFromPositions =
      sumEventAmounts(engine.events(), EventKind::SurplusReleased);
  const bool ok = releasedFromPositions == journal.releasedAmount;
  return makeCheck("journal_release_sum",
                   ok,
                   releasedFromPositions,
                   journal.releasedAmount,
                   ok ? "release events match journal" : "release event sum mismatch");
}

}  // namespace granite
