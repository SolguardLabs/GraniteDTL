#include "json.hpp"
#include "help.hpp"
#include "scenario.hpp"
#include "script.hpp"

#include <iostream>
#include <string>

namespace {

void printHelp() {
  std::cout << granite::versionText() << "\n"
            << "\n"
            << "Usage:\n"
            << "  granitedtl [scenario]\n"
            << "  granitedtl --list\n"
            << "  granitedtl script <file.gdtl>\n"
            << "\n"
            << "Build profile: " << granite::buildProfileText() << "\n"
            << "\n"
            << "Scenarios are emitted as deterministic JSON.\n"
            << "\n"
            << granite::commandHelpText();
}

}  // namespace

int main(int argc, char** argv) {
  using namespace granite;

  if (argc > 1) {
    const std::string first = argv[1];
    if (first == "--help" || first == "-h") {
      printHelp();
      return 0;
    }

    if (first == "--list" || first == "list") {
      writeScenarioList(std::cout, scenarioNames());
      return 0;
    }
  }

  Engine engine;

  if (argc > 1 && std::string(argv[1]) == "script") {
    if (argc < 3) {
      std::cerr << "script mode requires a file path\n";
      return 2;
    }

    ScriptRunner runner;
    const ScriptResult result = runner.runFile(argv[2], &engine);
    if (!result.ok) {
      std::cerr << "script error at line " << result.line << ": " << result.error << "\n";
      return 1;
    }

    writeReport(std::cout, engine);
    return 0;
  }

  const std::string scenario = argc > 1 ? argv[1] : "baseline";
  if (!runScenario(scenario, &engine)) {
    std::cerr << "unknown scenario: " << scenario << "\n";
    writeScenarioList(std::cerr, scenarioNames());
    return 1;
  }

  writeReport(std::cout, engine);
  return 0;
}
