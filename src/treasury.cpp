#include "treasury.hpp"

#include <array>

namespace granite {

namespace {

Amount addOrCap(Amount left, Amount right) {
  const MathResult result = AmountMath::add(left, right);
  return result.ok ? result.value : kMaxAmount;
}

Amount portion(Amount value, BasisPoints basisPoints) {
  const MathResult result = AmountMath::bps(value, basisPoints);
  return result.ok ? result.value : kMaxAmount;
}

bool validBps(BasisPoints value) {
  return value >= 0 && value <= kOneHundredPercentBps;
}

std::string classify(const TreasuryProjection& projection,
                     const StressPolicy& policy) {
  if (projection.liquidityGap > 0) {
    return "deficit";
  }

  if (projection.coverageBps >= 16'000 &&
      projection.laneConcentrationBps <= policy.concentrationLimitBps) {
    return "resilient";
  }

  if (projection.coverageBps >= 12'500 &&
      projection.laneConcentrationBps <= policy.concentrationLimitBps + 1'500) {
    return "guarded";
  }

  return "constrained";
}

}  // namespace

bool TreasuryStressModel::validInput(const TreasuryInput& input) {
  return input.reserveCash >= 0 && input.penaltyReserve >= 0 &&
         input.protocolFees >= 0 && input.lockedCollateral >= 0 &&
         input.openDebt >= 0 && input.maturingDebt >= 0 &&
         input.realizedInsolvency >= 0 && input.largestLaneDebt >= 0 &&
         input.maturingDebt <= input.openDebt &&
         input.largestLaneDebt <= input.openDebt;
}

bool TreasuryStressModel::validPolicy(const StressPolicy& policy) {
  return !policy.name.empty() && validBps(policy.reserveHaircutBps) &&
         validBps(policy.collateralHaircutBps) &&
         validBps(policy.maturityRunoffBps) &&
         validBps(policy.penaltyRecognitionBps) &&
         validBps(policy.feeRecognitionBps) &&
         validBps(policy.capitalBufferBps) &&
         validBps(policy.concentrationLimitBps);
}

TreasuryProjection TreasuryStressModel::project(const TreasuryInput& input,
                                                 const StressPolicy& policy) const {
  TreasuryProjection projection;
  projection.name = policy.name;
  projection.valid = validInput(input) && validPolicy(policy);
  projection.band = "invalid";

  if (!projection.valid) {
    return projection;
  }

  projection.stressedReserve =
      portion(input.reserveCash, kOneHundredPercentBps - policy.reserveHaircutBps);
  projection.eligibleCollateral =
      portion(input.lockedCollateral,
              kOneHundredPercentBps - policy.collateralHaircutBps);
  projection.recognizedPenaltyReserve =
      portion(input.penaltyReserve, policy.penaltyRecognitionBps);
  projection.recognizedFees = portion(input.protocolFees, policy.feeRecognitionBps);
  projection.maturingOutflow = portion(input.maturingDebt, policy.maturityRunoffBps);
  projection.capitalBuffer = portion(input.openDebt, policy.capitalBufferBps);

  projection.totalResources = addOrCap(
      addOrCap(projection.stressedReserve, projection.eligibleCollateral),
      addOrCap(projection.recognizedPenaltyReserve, projection.recognizedFees));
  projection.totalObligations =
      addOrCap(addOrCap(projection.maturingOutflow, projection.capitalBuffer),
               input.realizedInsolvency);

  if (projection.totalResources >= projection.totalObligations) {
    projection.netLiquidity =
        projection.totalResources - projection.totalObligations;
  } else {
    projection.liquidityGap =
        projection.totalObligations - projection.totalResources;
  }

  const RatioResult coverage =
      AmountMath::ratioBps(projection.totalResources, projection.totalObligations);
  projection.coverageBps = coverage.ok ? coverage.value : kMaxBasisPoints;

  if (input.openDebt > 0) {
    const RatioResult concentration =
        AmountMath::ratioBps(input.largestLaneDebt, input.openDebt);
    projection.laneConcentrationBps = concentration.ok ? concentration.value : 0;
  }

  projection.band = classify(projection, policy);
  return projection;
}

std::vector<TreasuryProjection> TreasuryStressModel::matrix(
    const TreasuryInput& input) const {
  const std::array<StressPolicy, 4> policies = {
      StressPolicy{"base", 0, 1'500, 8'500, 9'000, 10'000, 500, 4'000},
      StressPolicy{"liquidity-squeeze", 2'000, 2'500, 10'000, 7'000, 8'000,
                   800, 3'500},
      StressPolicy{"collateral-drawdown", 1'000, 4'500, 9'500, 5'000, 6'000,
                   1'200, 3'000},
      StressPolicy{"combined", 3'500, 6'500, 10'000, 3'000, 4'000, 1'800,
                   2'500},
  };

  std::vector<TreasuryProjection> projections;
  projections.reserve(policies.size());
  for (const StressPolicy& policy : policies) {
    projections.push_back(project(input, policy));
  }
  return projections;
}

}  // namespace granite
