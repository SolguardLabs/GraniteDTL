#include "engine.hpp"

#include <algorithm>
#include <utility>

namespace granite {

Engine::Engine() {
  resetFixture();
}

void Engine::clear() {
  scenarioName_ = "empty";
  policy_ = Policy{};
  vault_ = Vault{};
  accounts_.clear();
  positions_.clear();
  events_.clear();
  risk_.clear();
  clock_ = 0;
  nextEventSeq_ = 1;
  lastError_.clear();
  externalInflows_ = 0;
  externalWithdrawals_ = 0;
}

void Engine::resetFixture() {
  clear();
  scenarioName_ = "fixture";

  addAccount("alice", "Primary operator", 3'200'000);
  addAccount("bob", "Secondary operator", 2'500'000);
  addAccount("merchant", "Merchant counterparty", 150'000);
  addAccount("carrier", "Long route counterparty", 80'000);
  addReserve(1'400'000, "genesis_reserve");

  openPosition("alpha", "alice", "merchant", 1'750'000, 1'000'000, 8, 700, "atlantic", "");
  openPosition("beta", "bob", "carrier", 1'530'000, 900'000, 12, 600, "granite", "");
}

void Engine::setScenarioName(const std::string& name) {
  scenarioName_ = name.empty() ? "unnamed" : name;
}

const std::string& Engine::scenarioName() const {
  return scenarioName_;
}

const Policy& Engine::policy() const {
  return policy_;
}

Policy& Engine::mutablePolicy() {
  return policy_;
}

const Vault& Engine::vault() const {
  return vault_;
}

const std::map<std::string, Account>& Engine::accounts() const {
  return accounts_;
}

const std::map<std::string, Position>& Engine::positions() const {
  return positions_;
}

const std::vector<Event>& Engine::events() const {
  return events_;
}

const RiskBook& Engine::riskBook() const {
  return risk_;
}

TimePoint Engine::now() const {
  return clock_;
}

const std::string& Engine::lastError() const {
  return lastError_;
}

Amount Engine::externalInflows() const {
  return externalInflows_;
}

Amount Engine::externalWithdrawals() const {
  return externalWithdrawals_;
}

bool Engine::addAccount(const std::string& id, const std::string& label, Amount cash) {
  if (id.empty()) {
    return reject("account id is empty", "", "", cash);
  }

  if (cash < 0) {
    return reject("account cash is negative", id, "", cash);
  }

  if (accounts_.find(id) != accounts_.end()) {
    return reject("account already exists", id, "", cash);
  }

  Account account;
  account.id = id;
  account.label = label.empty() ? id : label;
  account.cash = cash;
  accounts_[id] = account;
  externalInflows_ += cash;

  emit(EventKind::AccountAdded, id, "", "", cash, 0, "account_registered");
  return true;
}

bool Engine::addReserve(Amount amount, const std::string& detail) {
  if (amount < 0) {
    return reject("reserve amount is negative", "", "", amount);
  }

  const MathResult next = AmountMath::add(vault_.reserveCash, amount);
  if (!next.ok) {
    return reject("reserve overflow", "", "", amount);
  }

  vault_.reserveCash = next.value;
  externalInflows_ += amount;
  bumpVaultVersion();

  emit(EventKind::ReserveAdded, "", "", "", amount, 0, detail.empty() ? "reserve_added" : detail);
  return true;
}

bool Engine::openPosition(const std::string& id,
                          const std::string& owner,
                          const std::string& counterparty,
                          Amount collateral,
                          Amount debt,
                          TimePoint ttl,
                          BasisPoints penaltyBps,
                          const std::string& lane,
                          const std::string& parent) {
  if (id.empty()) {
    return reject("position id is empty", owner, id, collateral);
  }

  if (positions_.find(id) != positions_.end()) {
    return reject("position already exists", owner, id, collateral);
  }

  Account* ownerAccount = findAccount(owner);
  if (ownerAccount == nullptr) {
    return reject("owner account not found", owner, id, collateral);
  }

  if (findAccount(counterparty) == nullptr) {
    return reject("counterparty account not found", counterparty, id, debt);
  }

  if (collateral <= 0 || debt <= 0) {
    return reject("collateral and debt must be positive", owner, id, collateral);
  }

  if (collateral > policy_.maxSingleLock) {
    return reject("collateral exceeds max single lock", owner, id, collateral);
  }

  if (ownerAccount->cash < collateral) {
    return reject("owner has insufficient cash", owner, id, collateral);
  }

  const Amount required = requiredCollateralFor(debt, policy_.minCoverageBps);
  if (collateral < required) {
    return reject("initial coverage below minimum", owner, id, collateral);
  }

  ownerAccount->cash -= collateral;
  ownerAccount->collateralPosted += collateral;

  Position position;
  position.id = id;
  position.owner = owner;
  position.counterparty = counterparty;
  position.lane = lane.empty() ? "default" : lane;
  position.parent = parent;
  position.state = PositionState::Active;
  position.lockState = LockState::Timelocked;
  position.originalCollateral = collateral;
  position.collateral = collateral;
  position.debt = debt;
  position.reserveClaim = required;
  position.penaltyBps = penaltyBps == 0 ? policy_.defaultPenaltyBps : penaltyBps;
  position.openedAt = clock_;
  position.dueAt = clock_ + (ttl <= 0 ? policy_.defaultTtl : ttl);
  position.unlockEpoch = locks_.nextUnlockEpoch(position, policy_);
  position.lastRequiredCollateral = required;
  position.lastQuotedSurplus = quoteSurplus(collateral, required);
  position.cachedCoverageBps = quoteCoverageBps(collateral, debt);
  position.realCoverageBps = position.cachedCoverageBps;

  positions_[id] = position;
  vault_.lockedCollateral += collateral;
  bumpVaultVersion();

  Position& stored = positions_.at(id);
  refreshRiskInternal(stored, "open_position");
  emit(EventKind::PositionOpened, owner, id, stored.lane, collateral, stored.cachedCoverageBps,
       "position_opened");
  return true;
}

bool Engine::advance(TimePoint delta) {
  if (delta < 0) {
    return reject("cannot move clock backwards", "", "", delta);
  }

  clock_ += delta;
  emit(EventKind::ClockAdvanced, "", "", "", delta, 0, "clock_advanced");
  return expireDueLocks();
}

bool Engine::expireDueLocks() {
  bool ok = true;

  for (auto& item : positions_) {
    Position& position = item.second;
    if (position.state != PositionState::Active) {
      continue;
    }

    if (!locks_.shouldExpire(position, policy_, clock_)) {
      continue;
    }

    position.state = PositionState::Matured;
    position.lockState = LockState::Expired;
    position.breachNotified = true;
    emit(EventKind::LockExpired, position.owner, position.id, position.lane, 0, 0,
         "lock_expired");
  }

  return ok;
}

bool Engine::refreshRisk(const std::string& positionId) {
  Position* position = findPosition(positionId);
  if (position == nullptr) {
    return reject("position not found", "", positionId, 0);
  }

  refreshRiskInternal(*position, "manual_refresh");
  return true;
}

bool Engine::accruePenalty(const std::string& positionId) {
  Position* position = findPosition(positionId);
  if (position == nullptr) {
    return reject("position not found", "", positionId, 0);
  }

  return accruePenaltyInternal(*position, true, "manual_penalty");
}

bool Engine::releaseSurplus(const std::string& positionId) {
  Position* position = findPosition(positionId);
  if (position == nullptr) {
    return reject("position not found", "", positionId, 0);
  }

  const CoverageView view =
      risk_.cachedOrObserve(*position, policy_, clock_, vault_.version);
  return releaseSurplusFromView(*position, view, true, "manual_release");
}

bool Engine::completePosition(const std::string& positionId) {
  Position* position = findPosition(positionId);
  if (position == nullptr) {
    return reject("position not found", "", positionId, 0);
  }

  if (position->terminal) {
    return reject("position already terminal", position->owner, position->id, 0);
  }

  if (!locks_.canCompleteWithoutPenalty(*position, policy_, clock_)) {
    return reject("expired position requires settlement", position->owner, position->id, 0);
  }

  const Amount remaining = position->collateral;
  if (remaining > 0) {
    if (!returnCollateralToOwner(*position, remaining, "complete_return")) {
      return false;
    }
  }

  position->state = PositionState::Closed;
  position->lockState = LockState::Released;
  position->terminal = true;
  position->closedAt = clock_;
  risk_.invalidate(position->id);

  emit(EventKind::PositionCompleted, position->owner, position->id, position->lane, remaining, 0,
       "position_completed");
  return true;
}

bool Engine::settleExpired(const std::string& positionId, SettlementMode mode) {
  Position* position = findPosition(positionId);
  if (position == nullptr) {
    return reject("position not found", "", positionId, 0);
  }

  if (position->terminal) {
    return reject("position already terminal", position->owner, position->id, 0);
  }

  position->closeAttempts += 1;

  if (position->state == PositionState::Active) {
    expireDueLocks();
  }

  if (position->state == PositionState::Active) {
    return reject("position not expired", position->owner, position->id, 0);
  }

  CoverageView cachedBeforePenalty = refreshRiskInternal(*position, "settlement_pre_penalty");

  if (!accruePenaltyInternal(*position, false, "settlement_penalty")) {
    return false;
  }

  if (policy_.allowSurplusRelease && cachedBeforePenalty.healthy &&
      cachedBeforePenalty.surplus > 0) {
    if (!releaseSurplusFromView(*position, cachedBeforePenalty, false,
                                "settlement_cached_release")) {
      return false;
    }
  }

  CoverageView after = refreshRiskInternal(*position, "settlement_post_release");
  markDefaultIfNeeded(*position, after, toString(mode));
  return true;
}

bool Engine::liquidatePosition(const std::string& positionId) {
  Position* position = findPosition(positionId);
  if (position == nullptr) {
    return reject("position not found", "", positionId, 0);
  }

  if (position->terminal) {
    return reject("position already terminal", position->owner, position->id, 0);
  }

  accruePenaltyInternal(*position, true, "liquidation_penalty");

  const Amount claim = position->debt;
  if (!payClaimFromPosition(*position, claim, "liquidation_claim")) {
    return false;
  }

  if (position->collateral > 0) {
    returnCollateralToOwner(*position, position->collateral, "liquidation_residual");
  }

  position->state = PositionState::Liquidated;
  position->lockState = LockState::Seized;
  position->terminal = true;
  position->closedAt = clock_;
  risk_.invalidate(position->id);

  emit(EventKind::PositionLiquidated, position->owner, position->id, position->lane,
       position->claimPaid, 0, "position_liquidated");
  return true;
}

bool Engine::withdraw(const std::string& accountId, Amount amount) {
  Account* account = findAccount(accountId);
  if (account == nullptr) {
    return reject("account not found", accountId, "", amount);
  }

  if (amount < 0) {
    return reject("withdrawal amount is negative", accountId, "", amount);
  }

  if (account->cash < amount) {
    return reject("insufficient account cash", accountId, "", amount);
  }

  account->cash -= amount;
  account->externalWithdrawals += amount;
  externalWithdrawals_ += amount;
  emit(EventKind::Withdrawal, accountId, "", "", amount, 0, "external_withdrawal");
  return true;
}

bool Engine::runMaintenance() {
  expireDueLocks();

  std::vector<std::string> ids = listPositionIds();
  for (const std::string& id : ids) {
    Position* position = findPosition(id);
    if (position == nullptr || position->terminal) {
      continue;
    }

    if (position->state == PositionState::Matured ||
        position->state == PositionState::Defaulted) {
      if (!settleExpired(id, SettlementMode::Maintenance)) {
        return false;
      }
    }
  }

  return true;
}

CheckSummary Engine::checks() const {
  CheckSummary summary;

  for (const auto& item : accounts_) {
    const Account& account = item.second;
    summary.totalAccountCash += account.cash;
    if (account.cash < 0) {
      summary.noNegativeAccounts = false;
    }
  }

  for (const auto& item : positions_) {
    const Position& position = item.second;
    summary.totalPositionCollateral += position.collateral;
    if (position.collateral < 0) {
      summary.noNegativePositions = false;
    }

    if (!position.terminal && position.state == PositionState::Defaulted &&
        position.shortfall > 0) {
      summary.terminalCoverageOk = false;
    }
  }

  summary.totalSystemFunds = summary.totalAccountCash + summary.totalPositionCollateral +
                             vault_.reserveCash + vault_.penaltyReserve + vault_.systemFees;
  summary.computedFunds = externalInflows_ - externalWithdrawals_;
  summary.accountingOk = summary.totalSystemFunds == summary.computedFunds;
  summary.reserveFloorOk = vault_.reserveCash >= policy_.reserveFloor;
  return summary;
}

std::vector<std::string> Engine::listPositionIds() const {
  std::vector<std::string> ids;
  ids.reserve(positions_.size());

  for (const auto& item : positions_) {
    ids.push_back(item.first);
  }

  return ids;
}

Account* Engine::findAccount(const std::string& id) {
  const auto found = accounts_.find(id);
  if (found == accounts_.end()) {
    return nullptr;
  }

  return &found->second;
}

const Account* Engine::findAccount(const std::string& id) const {
  const auto found = accounts_.find(id);
  if (found == accounts_.end()) {
    return nullptr;
  }

  return &found->second;
}

Position* Engine::findPosition(const std::string& id) {
  const auto found = positions_.find(id);
  if (found == positions_.end()) {
    return nullptr;
  }

  return &found->second;
}

const Position* Engine::findPosition(const std::string& id) const {
  const auto found = positions_.find(id);
  if (found == positions_.end()) {
    return nullptr;
  }

  return &found->second;
}

bool Engine::reject(const std::string& detail,
                    const std::string& account,
                    const std::string& position,
                    Amount amount) {
  lastError_ = detail;
  emit(EventKind::Rejected, account, position, "", amount, 0, detail);
  return false;
}

Event& Engine::emit(EventKind kind,
                    const std::string& account,
                    const std::string& position,
                    const std::string& lane,
                    Amount amount,
                    BasisPoints bps,
                    const std::string& detail) {
  Event event;
  event.seq = nextEventSeq_++;
  event.at = clock_;
  event.kind = kind;
  event.account = account;
  event.position = position;
  event.lane = lane;
  event.amount = amount;
  event.bps = bps;
  event.detail = detail;
  events_.push_back(event);
  return events_.back();
}

CoverageView Engine::refreshRiskInternal(Position& position, const std::string& detail) {
  CoverageView view = risk_.refresh(position, policy_, clock_, vault_.version);
  position.cachedCoverageBps = view.coverageBps;
  position.realCoverageBps = view.coverageBps;
  position.lastRiskAt = clock_;
  position.lastQuotedSurplus = view.surplus;
  position.lastRequiredCollateral = view.requiredCollateral;
  position.shortfall = view.shortfall;

  emit(EventKind::RiskRefreshed, position.owner, position.id, position.lane, view.surplus,
       view.coverageBps, detail);
  return view;
}

bool Engine::accruePenaltyInternal(Position& position,
                                   bool refreshAfter,
                                   const std::string& detail) {
  const PenaltyQuote quote = locks_.quotePenalty(position, policy_, clock_);
  position.lastPenaltyQuote = quote.incrementalPenalty;

  if (!quote.applicable || quote.incrementalPenalty <= 0) {
    if (refreshAfter) {
      refreshRiskInternal(position, detail + "_noop_refresh");
    }
    return true;
  }

  const Amount applied = AmountMath::min(quote.incrementalPenalty, position.collateral);
  if (applied <= 0) {
    return true;
  }

  position.collateral -= applied;
  position.penaltyAccrued += applied;
  position.lastPenaltyAt = clock_;
  position.state = position.state == PositionState::Active ? PositionState::Matured : position.state;

  Account* owner = findAccount(position.owner);
  if (owner != nullptr) {
    owner->penaltiesPaid += applied;
  }

  vault_.lockedCollateral -= applied;
  vault_.penaltyReserve += applied;
  bumpVaultVersion();

  emit(EventKind::PenaltyAccrued, position.owner, position.id, position.lane, applied,
       quote.effectiveBps, detail);

  if (refreshAfter) {
    refreshRiskInternal(position, detail + "_refresh");
  }

  return true;
}

bool Engine::releaseSurplusFromView(Position& position,
                                    const CoverageView& view,
                                    bool refreshAfter,
                                    const std::string& detail) {
  if (!policy_.allowSurplusRelease) {
    return reject("surplus release disabled", position.owner, position.id, 0);
  }

  if (!view.healthy) {
    return reject("coverage view is not healthy", position.owner, position.id, view.shortfall);
  }

  if (view.surplus <= 0) {
    if (refreshAfter) {
      refreshRiskInternal(position, detail + "_noop_refresh");
    }
    return true;
  }

  const Amount gross = AmountMath::min(view.surplus, position.collateral);
  if (gross <= 0) {
    return true;
  }

  return returnCollateralToOwner(position, gross, detail) &&
         (!refreshAfter || refreshRisk(position.id));
}

bool Engine::returnCollateralToOwner(Position& position,
                                     Amount amount,
                                     const std::string& detail) {
  if (amount < 0) {
    return reject("return amount is negative", position.owner, position.id, amount);
  }

  if (amount == 0) {
    return true;
  }

  if (position.collateral < amount) {
    return reject("position collateral too small", position.owner, position.id, amount);
  }

  Account* owner = findAccount(position.owner);
  if (owner == nullptr) {
    return reject("owner account not found", position.owner, position.id, amount);
  }

  const MathResult feeQuote = AmountMath::bps(amount, policy_.releaseFeeBps);
  if (!feeQuote.ok) {
    return reject("fee quote overflow", position.owner, position.id, amount);
  }

  const Amount fee = AmountMath::min(feeQuote.value, amount);
  const Amount net = amount - fee;

  position.collateral -= amount;
  position.surplusReleased += net;
  position.releaseFees += fee;
  position.lastQuotedSurplus = 0;

  owner->cash += net;
  owner->collateralReturned += net;
  owner->surplusWithdrawn += net;
  owner->feesPaid += fee;

  vault_.lockedCollateral -= amount;
  vault_.releasedSurplus += net;
  vault_.systemFees += fee;
  bumpVaultVersion();

  emit(EventKind::SurplusReleased, position.owner, position.id, position.lane, net,
       policy_.releaseFeeBps, detail);
  return true;
}

bool Engine::payClaimFromPosition(Position& position,
                                  Amount claimAmount,
                                  const std::string& detail) {
  Account* counterparty = findAccount(position.counterparty);
  if (counterparty == nullptr) {
    return reject("counterparty account not found", position.counterparty, position.id, claimAmount);
  }

  Amount remaining = claimAmount;
  Amount paidFromCollateral = 0;
  Amount paidFromReserve = 0;

  if (remaining > 0 && position.collateral > 0) {
    paidFromCollateral = AmountMath::min(position.collateral, remaining);
    position.collateral -= paidFromCollateral;
    vault_.lockedCollateral -= paidFromCollateral;
    remaining -= paidFromCollateral;
  }

  if (remaining > 0 && vault_.reserveCash > 0) {
    paidFromReserve = AmountMath::min(vault_.reserveCash, remaining);
    vault_.reserveCash -= paidFromReserve;
    remaining -= paidFromReserve;
  }

  const Amount paid = paidFromCollateral + paidFromReserve;
  counterparty->cash += paid;
  counterparty->claimsReceived += paid;
  position.claimPaid += paid;
  vault_.paidClaims += paid;

  if (remaining > 0) {
    position.shortfall += remaining;
    vault_.insolvency += remaining;
  }

  bumpVaultVersion();
  emit(EventKind::ClaimPaid, position.counterparty, position.id, position.lane, paid, 0, detail);
  return true;
}

bool Engine::markDefaultIfNeeded(Position& position,
                                 const CoverageView& view,
                                 const std::string& detail) {
  if (view.shortfall > 0) {
    position.state = PositionState::Defaulted;
    position.lockState = LockState::Expired;
    position.shortfall = view.shortfall;
    vault_.insolvency += view.shortfall;
    emit(EventKind::PositionDefaulted, position.owner, position.id, position.lane, view.shortfall,
         view.coverageBps, detail + "_coverage_shortfall");
    return true;
  }

  position.state = PositionState::Matured;
  position.shortfall = 0;
  emit(EventKind::ScriptNote, position.owner, position.id, position.lane, view.surplus,
       view.coverageBps, detail + "_settlement_healthy");
  return false;
}

void Engine::bumpVaultVersion() {
  vault_.version += 1;
}

}  // namespace granite
