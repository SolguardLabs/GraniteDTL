#pragma once

#include "engine.hpp"

#include <string>
#include <vector>

namespace granite {

std::vector<std::string> scenarioNames();

bool runScenario(const std::string& name, Engine* engine);

void configureLeanDefault(Engine* engine);

void configureChainFixture(Engine* engine);

}  // namespace granite
