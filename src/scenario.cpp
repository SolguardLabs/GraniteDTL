#include "scenario.hpp"

#include <algorithm>

namespace granite {

namespace {

bool runSnapshot(Engine* engine) {
  engine->resetFixture();
  engine->setScenarioName("snapshot");
  return true;
}

bool runBaseline(Engine* engine) {
  engine->resetFixture();
  engine->setScenarioName("baseline");
  engine->refreshRisk("alpha");
  engine->refreshRisk("beta");
  return true;
}

bool runHealthyClose(Engine* engine) {
  engine->resetFixture();
  engine->setScenarioName("healthy-close");
  engine->advance(3);
  engine->completePosition("alpha");
  engine->refreshRisk("beta");
  return true;
}

bool runSurplusRelease(Engine* engine) {
  engine->resetFixture();
  engine->setScenarioName("surplus-release");
  engine->refreshRisk("alpha");
  engine->releaseSurplus("alpha");
  engine->refreshRisk("beta");
  return true;
}

bool runExpiry(Engine* engine) {
  engine->resetFixture();
  engine->setScenarioName("expiry");
  engine->advance(11);
  engine->accruePenalty("alpha");
  engine->refreshRisk("alpha");
  return true;
}

bool runDefault(Engine* engine) {
  configureLeanDefault(engine);
  engine->setScenarioName("default");
  engine->advance(6);
  engine->settleExpired("lean", SettlementMode::Manual);
  return true;
}

bool runLiquidation(Engine* engine) {
  configureLeanDefault(engine);
  engine->setScenarioName("liquidation");
  engine->advance(6);
  engine->settleExpired("lean", SettlementMode::Manual);
  engine->liquidatePosition("lean");
  return true;
}

bool runMaintenance(Engine* engine) {
  configureChainFixture(engine);
  engine->setScenarioName("maintenance");
  engine->advance(7);
  engine->runMaintenance();
  return true;
}

bool runChain(Engine* engine) {
  configureChainFixture(engine);
  engine->setScenarioName("chain");
  engine->advance(4);
  engine->completePosition("north-a");
  engine->advance(3);
  engine->settleExpired("north-b", SettlementMode::Manual);
  engine->releaseSurplus("south-a");
  return true;
}

bool runWithdrawal(Engine* engine) {
  engine->resetFixture();
  engine->setScenarioName("withdrawal");
  engine->releaseSurplus("alpha");
  engine->withdraw("alice", 40'000);
  return true;
}

bool runReserveTopup(Engine* engine) {
  configureLeanDefault(engine);
  engine->setScenarioName("reserve-topup");
  engine->advance(6);
  engine->settleExpired("lean", SettlementMode::Manual);
  engine->addReserve(250'000, "operator_topup");
  engine->liquidatePosition("lean");
  return true;
}

}  // namespace

std::vector<std::string> scenarioNames() {
  return {
      "snapshot",
      "baseline",
      "healthy-close",
      "surplus-release",
      "expiry",
      "default",
      "liquidation",
      "maintenance",
      "chain",
      "withdrawal",
      "reserve-topup",
  };
}

bool runScenario(const std::string& name, Engine* engine) {
  if (engine == nullptr) {
    return false;
  }

  const std::string normalized = lowerCopy(name.empty() ? "baseline" : name);

  if (normalized == "snapshot") {
    return runSnapshot(engine);
  }

  if (normalized == "baseline") {
    return runBaseline(engine);
  }

  if (normalized == "healthy-close") {
    return runHealthyClose(engine);
  }

  if (normalized == "surplus-release") {
    return runSurplusRelease(engine);
  }

  if (normalized == "expiry") {
    return runExpiry(engine);
  }

  if (normalized == "default") {
    return runDefault(engine);
  }

  if (normalized == "liquidation") {
    return runLiquidation(engine);
  }

  if (normalized == "maintenance") {
    return runMaintenance(engine);
  }

  if (normalized == "chain") {
    return runChain(engine);
  }

  if (normalized == "withdrawal") {
    return runWithdrawal(engine);
  }

  if (normalized == "reserve-topup") {
    return runReserveTopup(engine);
  }

  return false;
}

void configureLeanDefault(Engine* engine) {
  engine->clear();
  engine->setScenarioName("lean-fixture");
  engine->addAccount("alice", "Lean operator", 2'000'000);
  engine->addAccount("merchant", "Merchant counterparty", 100'000);
  engine->addReserve(650'000, "lean_reserve");
  engine->openPosition("lean", "alice", "merchant", 1'500'000, 1'000'000, 2, 1'000,
                       "thin", "");
}

void configureChainFixture(Engine* engine) {
  engine->clear();
  engine->setScenarioName("chain-fixture");
  engine->addAccount("alice", "Northern operator", 4'000'000);
  engine->addAccount("bob", "Southern operator", 3'000'000);
  engine->addAccount("merchant", "Merchant counterparty", 180'000);
  engine->addAccount("carrier", "Carrier counterparty", 90'000);
  engine->addReserve(1'600'000, "chain_reserve");

  engine->openPosition("north-a", "alice", "merchant", 1'680'000, 1'000'000, 5, 650,
                       "north", "");
  engine->openPosition("north-b", "alice", "carrier", 1'620'000, 1'000'000, 4, 700,
                       "north", "north-a");
  engine->openPosition("south-a", "bob", "merchant", 1'950'000, 1'100'000, 9, 500,
                       "south", "");
}

}  // namespace granite
