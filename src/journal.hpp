#pragma once

#include "types.hpp"

#include <map>
#include <string>
#include <vector>

namespace granite {

struct JournalStats {
  std::map<std::string, std::int64_t> byKind;
  std::map<std::string, std::int64_t> byPosition;
  std::map<std::string, Amount> amountsByKind;
  std::vector<std::string> rejectionDetails;
  std::int64_t totalEvents = 0;
  std::int64_t rejectedEvents = 0;
  std::int64_t riskRefreshes = 0;
  std::int64_t releases = 0;
  std::int64_t penalties = 0;
  Amount releasedAmount = 0;
  Amount penaltyAmount = 0;
  Amount claimAmount = 0;
  Amount withdrawalAmount = 0;
};

JournalStats analyzeJournal(const std::vector<Event>& events);

std::vector<Event> eventsForPosition(const std::vector<Event>& events,
                                     const std::string& positionId);

std::vector<Event> eventsOfKind(const std::vector<Event>& events, EventKind kind);

bool journalHasKind(const std::vector<Event>& events, EventKind kind);

Amount sumEventAmounts(const std::vector<Event>& events, EventKind kind);

}  // namespace granite
