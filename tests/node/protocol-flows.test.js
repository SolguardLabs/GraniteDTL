import test from "node:test";
import assert from "node:assert/strict";

import { runJson } from "../helpers/granite-cli.js";

test("lists built-in scenarios", () => {
  const report = runJson(["--list"]);
  assert.ok(report.scenarios.includes("baseline"));
  assert.ok(report.scenarios.includes("surplus-release"));
  assert.ok(report.scenarios.includes("liquidation"));
});

test("baseline fixture is covered and accounting-stable", () => {
  const report = runJson(["baseline"]);
  assert.equal(report.scenario, "baseline");
  assert.equal(report.checks.accounting_ok, true);
  assert.equal(report.checks.no_negative_accounts, true);
  assert.equal(report.positions.alpha.state, "active");
  assert.equal(report.positions.beta.state, "active");
  assert.equal(report.audit.defaulted_positions, 0);
  assert.equal(report.invariants.ok, true);
});

test("surplus release moves only excess collateral to the owner", () => {
  const report = runJson(["surplus-release"]);
  const alpha = report.positions.alpha;

  assert.equal(alpha.collateral, 1_500_000);
  assert.equal(alpha.real_coverage_bps, 15_000);
  assert.equal(alpha.surplus_released, 249_375);
  assert.equal(alpha.release_fees, 625);
  assert.equal(report.vault.released_surplus, 249_375);
  assert.equal(report.checks.accounting_ok, true);
});

test("healthy close returns collateral before expiry", () => {
  const report = runJson(["healthy-close"]);
  const alpha = report.positions.alpha;

  assert.equal(alpha.state, "closed");
  assert.equal(alpha.terminal, true);
  assert.equal(alpha.collateral, 0);
  assert.ok(report.accounts.alice.cash > 3_000_000);
  assert.equal(report.checks.accounting_ok, true);
});
