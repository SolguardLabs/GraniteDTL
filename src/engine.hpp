#pragma once

#include "locks.hpp"
#include "risk.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace granite {

class Engine {
 public:
  Engine();

  void clear();

  void resetFixture();

  void setScenarioName(const std::string& name);

  const std::string& scenarioName() const;

  const Policy& policy() const;

  Policy& mutablePolicy();

  const Vault& vault() const;

  const std::map<std::string, Account>& accounts() const;

  const std::map<std::string, Position>& positions() const;

  const std::vector<Event>& events() const;

  const RiskBook& riskBook() const;

  TimePoint now() const;

  const std::string& lastError() const;

  Amount externalInflows() const;

  Amount externalWithdrawals() const;

  bool addAccount(const std::string& id, const std::string& label, Amount cash);

  bool addReserve(Amount amount, const std::string& detail);

  bool openPosition(const std::string& id,
                    const std::string& owner,
                    const std::string& counterparty,
                    Amount collateral,
                    Amount debt,
                    TimePoint ttl,
                    BasisPoints penaltyBps,
                    const std::string& lane,
                    const std::string& parent);

  bool advance(TimePoint delta);

  bool expireDueLocks();

  bool refreshRisk(const std::string& positionId);

  bool accruePenalty(const std::string& positionId);

  bool releaseSurplus(const std::string& positionId);

  bool completePosition(const std::string& positionId);

  bool settleExpired(const std::string& positionId, SettlementMode mode);

  bool liquidatePosition(const std::string& positionId);

  bool withdraw(const std::string& accountId, Amount amount);

  bool runMaintenance();

  CheckSummary checks() const;

  std::vector<std::string> listPositionIds() const;

 private:
  Account* findAccount(const std::string& id);

  const Account* findAccount(const std::string& id) const;

  Position* findPosition(const std::string& id);

  const Position* findPosition(const std::string& id) const;

  bool reject(const std::string& detail,
              const std::string& account,
              const std::string& position,
              Amount amount);

  Event& emit(EventKind kind,
              const std::string& account,
              const std::string& position,
              const std::string& lane,
              Amount amount,
              BasisPoints bps,
              const std::string& detail);

  CoverageView refreshRiskInternal(Position& position, const std::string& detail);

  bool accruePenaltyInternal(Position& position, bool refreshAfter, const std::string& detail);

  bool releaseSurplusFromView(Position& position,
                              const CoverageView& view,
                              bool refreshAfter,
                              const std::string& detail);

  bool returnCollateralToOwner(Position& position, Amount amount, const std::string& detail);

  bool payClaimFromPosition(Position& position, Amount claimAmount, const std::string& detail);

  bool markDefaultIfNeeded(Position& position, const CoverageView& view, const std::string& detail);

  void bumpVaultVersion();

  std::string scenarioName_ = "default";
  Policy policy_;
  Vault vault_;
  std::map<std::string, Account> accounts_;
  std::map<std::string, Position> positions_;
  std::vector<Event> events_;
  RiskBook risk_;
  LockBook locks_;
  TimePoint clock_ = 0;
  std::int64_t nextEventSeq_ = 1;
  std::string lastError_;
  Amount externalInflows_ = 0;
  Amount externalWithdrawals_ = 0;
};

}  // namespace granite
