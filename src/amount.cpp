#include "amount.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>

namespace granite {

MathResult AmountMath::add(Amount left, Amount right) {
  if (right > 0 && left > kMaxAmount - right) {
    return {};
  }

  if (right < 0 && left < -kMaxAmount - right) {
    return {};
  }

  return {true, left + right};
}

MathResult AmountMath::sub(Amount left, Amount right) {
  if (right < 0 && left > kMaxAmount + right) {
    return {};
  }

  if (right > 0 && left < -kMaxAmount + right) {
    return {};
  }

  return {true, left - right};
}

MathResult AmountMath::mul(Amount left, Amount right) {
  if (left == 0 || right == 0) {
    return {true, 0};
  }

  const Amount absLeft = left < 0 ? -left : left;
  const Amount absRight = right < 0 ? -right : right;

  if (absLeft > kMaxAmount / absRight) {
    return {};
  }

  return {true, left * right};
}

MathResult AmountMath::mulDiv(Amount value, Amount multiplier, Amount divisor) {
  if (divisor == 0) {
    return {};
  }

  const MathResult product = mul(value, multiplier);
  if (!product.ok) {
    return {};
  }

  return {true, product.value / divisor};
}

MathResult AmountMath::bps(Amount value, BasisPoints bps) {
  if (bps < 0 || bps > kMaxBasisPoints) {
    return {};
  }

  return mulDiv(value, bps, kOneHundredPercentBps);
}

RatioResult AmountMath::ratioBps(Amount numerator, Amount denominator) {
  if (denominator <= 0) {
    return {};
  }

  const MathResult scaled = mulDiv(numerator, kOneHundredPercentBps, denominator);
  if (!scaled.ok) {
    return {};
  }

  return {true, scaled.value};
}

Amount AmountMath::clampNonNegative(Amount value) {
  return value < 0 ? 0 : value;
}

Amount AmountMath::min(Amount left, Amount right) {
  return left < right ? left : right;
}

Amount AmountMath::max(Amount left, Amount right) {
  return left > right ? left : right;
}

bool AmountMath::isNonNegative(Amount value) {
  return value >= 0;
}

bool AmountMath::isPositive(Amount value) {
  return value > 0;
}

std::string formatAmount(Amount amount) {
  return std::to_string(amount);
}

std::string formatBps(BasisPoints bps) {
  std::ostringstream out;
  out << (bps / 100) << ".";
  const BasisPoints decimals = bps % 100;
  if (decimals < 10) {
    out << "0";
  }
  out << decimals << "%";
  return out.str();
}

bool parseAmount(const std::string& text, Amount* out) {
  if (text.empty() || out == nullptr) {
    return false;
  }

  char* end = nullptr;
  const long long parsed = std::strtoll(text.c_str(), &end, 10);
  if (end == text.c_str() || *end != '\0') {
    return false;
  }

  if (parsed < -kMaxAmount || parsed > kMaxAmount) {
    return false;
  }

  *out = static_cast<Amount>(parsed);
  return true;
}

bool parseBps(const std::string& text, BasisPoints* out) {
  Amount parsed = 0;
  if (!parseAmount(text, &parsed)) {
    return false;
  }

  if (parsed < 0 || parsed > kMaxBasisPoints) {
    return false;
  }

  if (out != nullptr) {
    *out = parsed;
  }

  return true;
}

std::vector<std::string> splitWords(const std::string& line) {
  std::vector<std::string> words;
  std::istringstream stream(line);
  std::string word;

  while (stream >> word) {
    words.push_back(word);
  }

  return words;
}

std::string trimCopy(const std::string& value) {
  std::size_t first = 0;
  while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first])) != 0) {
    first++;
  }

  std::size_t last = value.size();
  while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1])) != 0) {
    last--;
  }

  return value.substr(first, last - first);
}

std::string lowerCopy(const std::string& value) {
  std::string lowered = value;
  std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return lowered;
}

}  // namespace granite
