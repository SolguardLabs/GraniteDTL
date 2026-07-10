#pragma once

#include "audit.hpp"
#include "journal.hpp"

#include <string>
#include <vector>

namespace granite {

struct InvariantCheck {
  std::string name;
  bool ok = true;
  Amount expected = 0;
  Amount observed = 0;
  std::string detail;
};

struct InvariantReport {
  std::vector<InvariantCheck> checks;
  bool ok = true;
};

InvariantReport evaluateInvariants(const Engine& engine,
                                   const AuditSummary& audit,
                                   const JournalStats& journal);

InvariantCheck checkAccounting(const Engine& engine);

InvariantCheck checkReserveFloor(const Engine& engine);

InvariantCheck checkNoNegativeBalances(const Engine& engine);

InvariantCheck checkOpenCoverage(const Engine& engine, const AuditSummary& audit);

InvariantCheck checkJournalConsistency(const Engine& engine, const JournalStats& journal);

}  // namespace granite
