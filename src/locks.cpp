#include "locks.hpp"

namespace granite {

LockQuote LockBook::quote(const Position& position, const Policy& policy, TimePoint now) const {
  LockQuote quote;
  quote.open = isOpen(position.state);
  quote.unlockEpoch = nextUnlockEpoch(position, policy);

  if (!quote.open) {
    quote.reason = "terminal";
    return quote;
  }

  if (now <= position.dueAt) {
    quote.remaining = position.dueAt - now;
    quote.reason = "before_due";
    return quote;
  }

  quote.overdue = now - position.dueAt;
  quote.expired = quote.overdue > policy.gracePeriod;
  quote.inGrace = !quote.expired;
  quote.reason = quote.expired ? "expired" : "grace";
  return quote;
}

PenaltyQuote LockBook::quotePenalty(const Position& position,
                                    const Policy& policy,
                                    TimePoint now) const {
  PenaltyQuote quote;

  if (!isOpen(position.state)) {
    return quote;
  }

  if (now <= position.dueAt + policy.gracePeriod) {
    return quote;
  }

  quote.applicable = true;
  quote.lateness = now - position.dueAt - policy.gracePeriod;

  BasisPoints effectiveBps = position.penaltyBps;
  if (effectiveBps == 0) {
    effectiveBps = policy.defaultPenaltyBps;
  }

  const MathResult daily = AmountMath::mul(quote.lateness, policy.dailyPenaltyBps);
  if (daily.ok) {
    effectiveBps += daily.value;
  } else {
    effectiveBps = policy.maxPenaltyBps;
  }

  if (effectiveBps > policy.maxPenaltyBps) {
    effectiveBps = policy.maxPenaltyBps;
    quote.capped = true;
  }

  quote.effectiveBps = effectiveBps;

  const MathResult target = AmountMath::bps(position.originalCollateral, effectiveBps);
  if (!target.ok) {
    return quote;
  }

  quote.targetPenalty = AmountMath::min(target.value, position.collateral + position.penaltyAccrued);

  if (quote.targetPenalty > position.penaltyAccrued) {
    quote.incrementalPenalty = quote.targetPenalty - position.penaltyAccrued;
  }

  return quote;
}

bool LockBook::shouldExpire(const Position& position, const Policy& policy, TimePoint now) const {
  return quote(position, policy, now).expired;
}

bool LockBook::canCompleteWithoutPenalty(const Position& position,
                                         const Policy& policy,
                                         TimePoint now) const {
  const LockQuote quoted = quote(position, policy, now);
  return quoted.open && !quoted.expired;
}

TimePoint LockBook::nextUnlockEpoch(const Position& position, const Policy& policy) const {
  if (position.dueAt <= 0) {
    return policy.defaultTtl;
  }

  return position.dueAt + policy.gracePeriod;
}

std::vector<std::string> LockBook::explain(const Position& position,
                                           const Policy& policy,
                                           TimePoint now) const {
  const LockQuote lock = quote(position, policy, now);
  std::vector<std::string> lines;

  lines.push_back("position=" + position.id);
  lines.push_back("state=" + toString(position.state));
  lines.push_back("lock=" + toString(position.lockState));
  lines.push_back("reason=" + lock.reason);
  lines.push_back("remaining=" + std::to_string(lock.remaining));
  lines.push_back("overdue=" + std::to_string(lock.overdue));
  lines.push_back("unlock_epoch=" + std::to_string(lock.unlockEpoch));

  const PenaltyQuote penalty = quotePenalty(position, policy, now);
  lines.push_back("penalty_applicable=" + std::string(penalty.applicable ? "true" : "false"));
  lines.push_back("penalty_bps=" + std::to_string(penalty.effectiveBps));
  lines.push_back("incremental_penalty=" + std::to_string(penalty.incrementalPenalty));

  return lines;
}

}  // namespace granite
