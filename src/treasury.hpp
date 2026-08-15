#pragma once

#include "amount.hpp"

#include <string>
#include <vector>

namespace granite {

struct TreasuryInput {
  Amount reserveCash = 0;
  Amount penaltyReserve = 0;
  Amount protocolFees = 0;
  Amount lockedCollateral = 0;
  Amount openDebt = 0;
  Amount maturingDebt = 0;
  Amount realizedInsolvency = 0;
  Amount largestLaneDebt = 0;
};

struct StressPolicy {
  std::string name;
  BasisPoints reserveHaircutBps = 0;
  BasisPoints collateralHaircutBps = 0;
  BasisPoints maturityRunoffBps = 0;
  BasisPoints penaltyRecognitionBps = 0;
  BasisPoints feeRecognitionBps = 0;
  BasisPoints capitalBufferBps = 0;
  BasisPoints concentrationLimitBps = 0;
};

struct TreasuryProjection {
  std::string name;
  bool valid = false;
  Amount stressedReserve = 0;
  Amount eligibleCollateral = 0;
  Amount recognizedPenaltyReserve = 0;
  Amount recognizedFees = 0;
  Amount maturingOutflow = 0;
  Amount capitalBuffer = 0;
  Amount totalResources = 0;
  Amount totalObligations = 0;
  Amount netLiquidity = 0;
  Amount liquidityGap = 0;
  BasisPoints coverageBps = 0;
  BasisPoints laneConcentrationBps = 0;
  std::string band;
};

class TreasuryStressModel {
 public:
  TreasuryProjection project(const TreasuryInput& input,
                             const StressPolicy& policy) const;

  std::vector<TreasuryProjection> matrix(const TreasuryInput& input) const;

  static bool validInput(const TreasuryInput& input);

  static bool validPolicy(const StressPolicy& policy);
};

}  // namespace granite
