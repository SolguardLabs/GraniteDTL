import test from "node:test";
import assert from "node:assert/strict";

import { runJson } from "../helpers/granite-cli.js";

test("expired lock accrues penalty and remains observable", () => {
  const report = runJson(["expiry"]);
  const alpha = report.positions.alpha;

  assert.equal(alpha.state, "matured");
  assert.equal(alpha.lock_state, "expired");
  assert.ok(alpha.penalty_accrued > 0);
  assert.ok(alpha.real_coverage_bps >= report.policy.min_coverage_bps);
  assert.equal(report.journal.penalties, 1);
  assert.equal(report.checks.accounting_ok, true);
});

test("thin expired position becomes defaulted after settlement", () => {
  const report = runJson(["default"]);
  const lean = report.positions.lean;

  assert.equal(lean.state, "defaulted");
  assert.ok(lean.shortfall > 0);
  assert.equal(report.invariants.ok, false);
  assert.equal(report.plan.has_liquidations, true);
  assert.ok(report.audit.warnings.length > 0);
});

test("liquidation pays the counterparty and terminates the lock", () => {
  const report = runJson(["liquidation"]);
  const lean = report.positions.lean;

  assert.equal(lean.state, "liquidated");
  assert.equal(lean.terminal, true);
  assert.ok(lean.claim_paid > 0);
  assert.ok(report.accounts.merchant.claims_received > 0);
  assert.equal(report.invariants.checks.find((check) => check.name === "open_coverage").ok, true);
});

test("maintenance processes matured positions deterministically", () => {
  const report = runJson(["maintenance"]);

  assert.equal(report.scenario, "maintenance");
  assert.ok(report.journal.risk_refreshes > 0);
  assert.equal(report.checks.no_negative_positions, true);
  assert.ok(report.portfolio.buckets.length >= 1);
});
