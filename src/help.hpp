#pragma once

#include <string>
#include <vector>

namespace granite {

struct CommandHelp {
  std::string name;
  std::string usage;
  std::string description;
};

std::vector<CommandHelp> scriptCommandHelp();

std::string commandHelpText();

std::string versionText();

std::string buildProfileText();

}  // namespace granite
