#pragma once

#include "engine.hpp"

#include <ostream>
#include <string>
#include <vector>

namespace granite {

class JsonWriter {
 public:
  explicit JsonWriter(std::ostream& out);

  void beginObject();

  void endObject();

  void beginArray();

  void endArray();

  void key(const std::string& name);

  void stringValue(const std::string& value);

  void amountValue(Amount value);

  void integerValue(std::int64_t value);

  void boolValue(bool value);

  void nullValue();

  void comma();

  void newline();

  void indent();

 private:
  std::ostream& out_;
  int depth_ = 0;
};

std::string jsonEscape(const std::string& value);

void writeReport(std::ostream& out, const Engine& engine);

void writeScenarioList(std::ostream& out, const std::vector<std::string>& names);

}  // namespace granite
