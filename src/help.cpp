#include "help.hpp"

#include <sstream>

namespace granite {

std::vector<CommandHelp> scriptCommandHelp() {
  return {
      {
          "SCENARIO",
          "SCENARIO <name>",
          "sets the report scenario label",
      },
      {
          "RESET",
          "RESET",
          "loads the deterministic fixture with operators, counterparties and locks",
      },
      {
          "EMPTY",
          "EMPTY",
          "clears all accounts, positions, reserves, events and risk views",
      },
      {
          "ACCOUNT",
          "ACCOUNT <id> <label> <cash>",
          "registers a local cash account",
      },
      {
          "RESERVE",
          "RESERVE <amount> [detail]",
          "adds external reserve cash to the guarantee vault",
      },
      {
          "OPEN",
          "OPEN <id> <owner> <counterparty> <collateral> <debt> <ttl> <penalty_bps> <lane> [parent]",
          "opens a timelocked collateral position",
      },
      {
          "ADVANCE",
          "ADVANCE <delta>",
          "moves the deterministic engine clock forward",
      },
      {
          "REFRESH",
          "REFRESH <position>",
          "recomputes the cached coverage view for one position",
      },
      {
          "PENALTY",
          "PENALTY <position>",
          "accrues the currently due expiration penalty and refreshes risk",
      },
      {
          "RELEASE",
          "RELEASE <position>",
          "releases collateral surplus according to the cached or observed view",
      },
      {
          "COMPLETE",
          "COMPLETE <position>",
          "closes an unexpired position and returns remaining collateral",
      },
      {
          "SETTLE",
          "SETTLE <position>",
          "runs the expiration settlement path for a matured lock",
      },
      {
          "LIQUIDATE",
          "LIQUIDATE <position>",
          "pays the counterparty claim from collateral and reserve cash",
      },
      {
          "WITHDRAW",
          "WITHDRAW <account> <amount>",
          "removes account cash from the local system model",
      },
      {
          "MAINTAIN",
          "MAINTAIN",
          "expires due locks and settles matured positions",
      },
      {
          "POLICY",
          "POLICY <key> <value>",
          "updates a runtime policy parameter for the current script",
      },
      {
          "RUN",
          "RUN <scenario>",
          "executes one built-in scenario from inside a script",
      },
      {
          "TRY_*",
          "TRY_<COMMAND> ...",
          "executes a command while keeping the script alive if it rejects",
      },
  };
}

std::string commandHelpText() {
  std::ostringstream out;
  out << "Script commands:\n";
  for (const CommandHelp& command : scriptCommandHelp()) {
    out << "  " << command.usage << "\n";
    out << "    " << command.description << "\n";
  }
  return out.str();
}

std::string versionText() {
  return "GraniteDTL 0.1.0";
}

std::string buildProfileText() {
  return "portable-cxx17";
}

}  // namespace granite
