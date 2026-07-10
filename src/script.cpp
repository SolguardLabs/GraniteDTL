#include "script.hpp"

#include "scenario.hpp"

#include <fstream>
#include <sstream>

namespace granite {

namespace {

bool parseRequiredAmount(const std::vector<std::string>& words,
                         std::size_t index,
                         const std::string& field,
                         Amount* out,
                         std::string* error) {
  if (index >= words.size()) {
    if (error != nullptr) {
      *error = "missing " + field;
    }
    return false;
  }

  if (!parseAmount(words[index], out)) {
    if (error != nullptr) {
      *error = "invalid " + field + ": " + words[index];
    }
    return false;
  }

  return true;
}

bool parseRequiredBps(const std::vector<std::string>& words,
                      std::size_t index,
                      const std::string& field,
                      BasisPoints* out,
                      std::string* error) {
  if (index >= words.size()) {
    if (error != nullptr) {
      *error = "missing " + field;
    }
    return false;
  }

  if (!parseBps(words[index], out)) {
    if (error != nullptr) {
      *error = "invalid " + field + ": " + words[index];
    }
    return false;
  }

  return true;
}

std::string commandWithoutTry(const std::string& command, bool* tryMode) {
  if (command.rfind("TRY_", 0) == 0) {
    if (tryMode != nullptr) {
      *tryMode = true;
    }
    return command.substr(4);
  }

  if (tryMode != nullptr) {
    *tryMode = false;
  }
  return command;
}

}  // namespace

ScriptResult ScriptRunner::runFile(const std::string& path, Engine* engine) const {
  std::ifstream input(path);
  if (!input) {
    return {false, 0, "could not open script: " + path};
  }

  return runStream(input, engine);
}

ScriptResult ScriptRunner::runStream(std::istream& input, Engine* engine) const {
  if (engine == nullptr) {
    return {false, 0, "engine is null"};
  }

  std::string line;
  int lineNumber = 0;
  while (std::getline(input, line)) {
    lineNumber++;
    std::string error;
    if (!execute(line, lineNumber, engine, &error)) {
      return {false, lineNumber, error};
    }
  }

  return {true, lineNumber, ""};
}

bool ScriptRunner::execute(const std::string& rawLine,
                           int lineNumber,
                           Engine* engine,
                           std::string* error) const {
  std::string line = trimCopy(rawLine);
  if (line.rfind("\xEF\xBB\xBF", 0) == 0) {
    line.erase(0, 3);
  }
  if (line.empty()) {
    return true;
  }

  if (line[0] == '#') {
    return true;
  }

  const std::vector<std::string> words = splitWords(line);
  if (words.empty()) {
    return true;
  }

  bool tryMode = false;
  const std::string command = commandWithoutTry(words[0], &tryMode);
  bool ok = false;

  if (command == "SCENARIO") {
    if (words.size() < 2) {
      if (error != nullptr) {
        *error = "SCENARIO requires a name";
      }
      return false;
    }
    engine->setScenarioName(words[1]);
    return true;
  }

  if (command == "RESET") {
    const std::string scenario = engine->scenarioName();
    engine->resetFixture();
    engine->setScenarioName(scenario);
    ok = true;
  } else if (command == "EMPTY") {
    const std::string scenario = engine->scenarioName();
    engine->clear();
    engine->setScenarioName(scenario);
    ok = true;
  } else if (command == "RUN") {
    if (words.size() < 2) {
      if (error != nullptr) {
        *error = "RUN requires a scenario name";
      }
      return false;
    }
    ok = runScenario(words[1], engine);
  } else if (command == "ACCOUNT") {
    Amount cash = 0;
    if (words.size() < 4 ||
        !parseRequiredAmount(words, 3, "cash", &cash, error)) {
      if (error != nullptr && error->empty()) {
        *error = "ACCOUNT <id> <label> <cash>";
      }
      return false;
    }
    ok = engine->addAccount(words[1], words[2], cash);
  } else if (command == "RESERVE") {
    Amount amount = 0;
    if (!parseRequiredAmount(words, 1, "amount", &amount, error)) {
      return false;
    }
    const std::string detail = words.size() > 2 ? words[2] : "script_reserve";
    ok = engine->addReserve(amount, detail);
  } else if (command == "OPEN") {
    Amount collateral = 0;
    Amount debt = 0;
    Amount ttl = 0;
    BasisPoints penaltyBps = 0;
    if (words.size() < 9 ||
        !parseRequiredAmount(words, 4, "collateral", &collateral, error) ||
        !parseRequiredAmount(words, 5, "debt", &debt, error) ||
        !parseRequiredAmount(words, 6, "ttl", &ttl, error) ||
        !parseRequiredBps(words, 7, "penalty_bps", &penaltyBps, error)) {
      if (error != nullptr && error->empty()) {
        *error = "OPEN <id> <owner> <counterparty> <collateral> <debt> <ttl> <penalty_bps> <lane> [parent]";
      }
      return false;
    }
    const std::string parent = words.size() > 9 ? words[9] : "";
    ok = engine->openPosition(words[1], words[2], words[3], collateral, debt, ttl, penaltyBps,
                              words[8], parent);
  } else if (command == "ADVANCE") {
    Amount delta = 0;
    if (!parseRequiredAmount(words, 1, "delta", &delta, error)) {
      return false;
    }
    ok = engine->advance(delta);
  } else if (command == "REFRESH") {
    if (words.size() < 2) {
      if (error != nullptr) {
        *error = "REFRESH requires a position id";
      }
      return false;
    }
    ok = engine->refreshRisk(words[1]);
  } else if (command == "PENALTY") {
    if (words.size() < 2) {
      if (error != nullptr) {
        *error = "PENALTY requires a position id";
      }
      return false;
    }
    ok = engine->accruePenalty(words[1]);
  } else if (command == "RELEASE") {
    if (words.size() < 2) {
      if (error != nullptr) {
        *error = "RELEASE requires a position id";
      }
      return false;
    }
    ok = engine->releaseSurplus(words[1]);
  } else if (command == "COMPLETE") {
    if (words.size() < 2) {
      if (error != nullptr) {
        *error = "COMPLETE requires a position id";
      }
      return false;
    }
    ok = engine->completePosition(words[1]);
  } else if (command == "SETTLE") {
    if (words.size() < 2) {
      if (error != nullptr) {
        *error = "SETTLE requires a position id";
      }
      return false;
    }
    ok = engine->settleExpired(words[1], SettlementMode::Scripted);
  } else if (command == "LIQUIDATE") {
    if (words.size() < 2) {
      if (error != nullptr) {
        *error = "LIQUIDATE requires a position id";
      }
      return false;
    }
    ok = engine->liquidatePosition(words[1]);
  } else if (command == "WITHDRAW") {
    Amount amount = 0;
    if (words.size() < 3 ||
        !parseRequiredAmount(words, 2, "amount", &amount, error)) {
      if (error != nullptr && error->empty()) {
        *error = "WITHDRAW <account> <amount>";
      }
      return false;
    }
    ok = engine->withdraw(words[1], amount);
  } else if (command == "MAINTAIN") {
    ok = engine->runMaintenance();
  } else if (command == "POLICY") {
    ok = applyPolicy(words, engine, error);
  } else {
    if (error != nullptr) {
      *error = "unknown command on line " + std::to_string(lineNumber) + ": " + command;
    }
    return false;
  }

  if (tryMode) {
    return true;
  }

  if (!ok && error != nullptr) {
    *error = engine->lastError().empty() ? "command failed: " + command : engine->lastError();
  }

  return ok;
}

bool ScriptRunner::applyPolicy(const std::vector<std::string>& words,
                               Engine* engine,
                               std::string* error) const {
  if (words.size() < 3) {
    if (error != nullptr) {
      *error = "POLICY <key> <value>";
    }
    return false;
  }

  const std::string key = lowerCopy(words[1]);
  Amount amount = 0;
  BasisPoints bps = 0;
  Policy& policy = engine->mutablePolicy();

  if (key == "min_coverage_bps") {
    if (!parseRequiredBps(words, 2, "min_coverage_bps", &bps, error)) {
      return false;
    }
    policy.minCoverageBps = bps;
    return true;
  }

  if (key == "target_coverage_bps") {
    if (!parseRequiredBps(words, 2, "target_coverage_bps", &bps, error)) {
      return false;
    }
    policy.targetCoverageBps = bps;
    return true;
  }

  if (key == "default_penalty_bps") {
    if (!parseRequiredBps(words, 2, "default_penalty_bps", &bps, error)) {
      return false;
    }
    policy.defaultPenaltyBps = bps;
    return true;
  }

  if (key == "daily_penalty_bps") {
    if (!parseRequiredBps(words, 2, "daily_penalty_bps", &bps, error)) {
      return false;
    }
    policy.dailyPenaltyBps = bps;
    return true;
  }

  if (key == "max_penalty_bps") {
    if (!parseRequiredBps(words, 2, "max_penalty_bps", &bps, error)) {
      return false;
    }
    policy.maxPenaltyBps = bps;
    return true;
  }

  if (key == "release_fee_bps") {
    if (!parseRequiredBps(words, 2, "release_fee_bps", &bps, error)) {
      return false;
    }
    policy.releaseFeeBps = bps;
    return true;
  }

  if (key == "grace_period") {
    if (!parseRequiredAmount(words, 2, "grace_period", &amount, error)) {
      return false;
    }
    policy.gracePeriod = amount;
    return true;
  }

  if (key == "reserve_floor") {
    if (!parseRequiredAmount(words, 2, "reserve_floor", &amount, error)) {
      return false;
    }
    policy.reserveFloor = amount;
    return true;
  }

  if (key == "max_single_lock") {
    if (!parseRequiredAmount(words, 2, "max_single_lock", &amount, error)) {
      return false;
    }
    policy.maxSingleLock = amount;
    return true;
  }

  if (key == "allow_surplus_release") {
    const std::string value = lowerCopy(words[2]);
    if (value == "true" || value == "1" || value == "yes") {
      policy.allowSurplusRelease = true;
      return true;
    }
    if (value == "false" || value == "0" || value == "no") {
      policy.allowSurplusRelease = false;
      return true;
    }
    if (error != nullptr) {
      *error = "invalid boolean for allow_surplus_release";
    }
    return false;
  }

  if (error != nullptr) {
    *error = "unknown policy key: " + words[1];
  }
  return false;
}

}  // namespace granite
