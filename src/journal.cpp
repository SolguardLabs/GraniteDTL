#include "journal.hpp"

namespace granite {

JournalStats analyzeJournal(const std::vector<Event>& events) {
  JournalStats stats;
  stats.totalEvents = static_cast<std::int64_t>(events.size());

  for (const Event& event : events) {
    const std::string kind = toString(event.kind);
    stats.byKind[kind] += 1;
    stats.amountsByKind[kind] += event.amount;

    if (!event.position.empty()) {
      stats.byPosition[event.position] += 1;
    }

    if (event.kind == EventKind::Rejected) {
      stats.rejectedEvents += 1;
      stats.rejectionDetails.push_back(event.detail);
    }

    if (event.kind == EventKind::RiskRefreshed) {
      stats.riskRefreshes += 1;
    }

    if (event.kind == EventKind::SurplusReleased) {
      stats.releases += 1;
      stats.releasedAmount += event.amount;
    }

    if (event.kind == EventKind::PenaltyAccrued) {
      stats.penalties += 1;
      stats.penaltyAmount += event.amount;
    }

    if (event.kind == EventKind::ClaimPaid) {
      stats.claimAmount += event.amount;
    }

    if (event.kind == EventKind::Withdrawal) {
      stats.withdrawalAmount += event.amount;
    }
  }

  return stats;
}

std::vector<Event> eventsForPosition(const std::vector<Event>& events,
                                     const std::string& positionId) {
  std::vector<Event> result;

  for (const Event& event : events) {
    if (event.position == positionId) {
      result.push_back(event);
    }
  }

  return result;
}

std::vector<Event> eventsOfKind(const std::vector<Event>& events, EventKind kind) {
  std::vector<Event> result;

  for (const Event& event : events) {
    if (event.kind == kind) {
      result.push_back(event);
    }
  }

  return result;
}

bool journalHasKind(const std::vector<Event>& events, EventKind kind) {
  for (const Event& event : events) {
    if (event.kind == kind) {
      return true;
    }
  }

  return false;
}

Amount sumEventAmounts(const std::vector<Event>& events, EventKind kind) {
  Amount total = 0;

  for (const Event& event : events) {
    if (event.kind == kind) {
      total += event.amount;
    }
  }

  return total;
}

}  // namespace granite
