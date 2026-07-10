#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace granite {

using Amount = std::int64_t;
using TimePoint = std::int64_t;
using BasisPoints = std::int64_t;

constexpr Amount kZeroAmount = 0;
constexpr BasisPoints kOneHundredPercentBps = 10'000;
constexpr BasisPoints kMaxBasisPoints = 100'000;
constexpr Amount kMaxAmount = std::numeric_limits<Amount>::max() / 4;

struct MathResult {
  bool ok = false;
  Amount value = 0;
};

struct RatioResult {
  bool ok = false;
  BasisPoints value = 0;
};

class AmountMath {
 public:
  static MathResult add(Amount left, Amount right);

  static MathResult sub(Amount left, Amount right);

  static MathResult mul(Amount left, Amount right);

  static MathResult mulDiv(Amount value, Amount multiplier, Amount divisor);

  static MathResult bps(Amount value, BasisPoints bps);

  static RatioResult ratioBps(Amount numerator, Amount denominator);

  static Amount clampNonNegative(Amount value);

  static Amount min(Amount left, Amount right);

  static Amount max(Amount left, Amount right);

  static bool isNonNegative(Amount value);

  static bool isPositive(Amount value);
};

std::string formatAmount(Amount amount);

std::string formatBps(BasisPoints bps);

bool parseAmount(const std::string& text, Amount* out);

bool parseBps(const std::string& text, BasisPoints* out);

std::vector<std::string> splitWords(const std::string& line);

std::string trimCopy(const std::string& value);

std::string lowerCopy(const std::string& value);

}  // namespace granite
