#pragma once

#include "engine.hpp"

#include <istream>
#include <string>

namespace granite {

struct ScriptResult {
  bool ok = false;
  int line = 0;
  std::string error;
};

class ScriptRunner {
 public:
  ScriptResult runFile(const std::string& path, Engine* engine) const;

  ScriptResult runStream(std::istream& input, Engine* engine) const;

 private:
  bool execute(const std::string& line,
               int lineNumber,
               Engine* engine,
               std::string* error) const;

  bool applyPolicy(const std::vector<std::string>& words,
                   Engine* engine,
                   std::string* error) const;
};

}  // namespace granite
