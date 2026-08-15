import test from "node:test";
import assert from "node:assert/strict";
import { mkdtempSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";

import { GraniteClient, GraniteExecutionError } from "../../sdk/client.js";
import { validateReport } from "../../sdk/report.js";
import { ensureBuilt } from "../helpers/granite-cli.js";

const binary = ensureBuilt();
const client = new GraniteClient({ binary, timeoutMs: 5_000 });

test("SDK lists the deterministic scenario registry", () => {
  const scenarios = client.listScenarios();
  assert.ok(scenarios.includes("baseline"));
  assert.ok(scenarios.includes("maintenance"));
  assert.equal(Object.isFrozen(scenarios), true);
});

test("SDK validates and freezes protocol reports", () => {
  const report = client.runScenario("baseline");
  assert.equal(report.scenario, "baseline");
  assert.equal(report.checks.accounting_ok, true);
  assert.equal(Object.isFrozen(report), true);
  assert.equal(Object.isFrozen(report.treasury_stress.scenarios), true);
});

test("stress matrix exposes four ordered policies", () => {
  const report = client.runScenario("baseline");
  const matrix = report.treasury_stress.scenarios;
  assert.deepEqual(
    matrix.map((scenario) => scenario.name),
    ["base", "liquidity-squeeze", "collateral-drawdown", "combined"],
  );
  assert.equal(
    matrix.every((scenario) => scenario.valid),
    true,
  );
});

test("combined stress recognizes fewer resources than base", () => {
  const report = client.runScenario("baseline");
  const [base, , , combined] = report.treasury_stress.scenarios;
  assert.ok(combined.total_resources < base.total_resources);
  assert.ok(combined.capital_buffer > base.capital_buffer);
  assert.ok(combined.coverage_bps < base.coverage_bps);
});

test("lane concentration is derived from open debt", () => {
  const report = client.runScenario("baseline");
  assert.equal(report.treasury_stress.open_debt, 1_900_000);
  assert.equal(report.treasury_stress.largest_lane_debt, 1_000_000);
  assert.equal(report.treasury_stress.scenarios[0].lane_concentration_bps, 5_263);
});

test("script execution keeps accounting and stress output available", () => {
  const directory = mkdtempSync(join(tmpdir(), "granite-sdk-"));
  const script = join(directory, "sdk-flow.gdtl");
  writeFileSync(
    script,
    [
      "SCENARIO sdk-flow",
      "EMPTY",
      "ACCOUNT owner Owner 2500000",
      "ACCOUNT merchant Merchant 100000",
      "RESERVE 900000 seed",
      "OPEN p1 owner merchant 1650000 1000000 8 600 lane-a",
      "ADVANCE 2",
      "REFRESH p1",
      "",
    ].join("\n"),
  );
  const report = client.runScript(script);
  assert.equal(report.scenario, "sdk-flow");
  assert.equal(report.checks.accounting_ok, true);
  assert.equal(report.treasury_stress.scenarios.length, 4);
});

test("SDK rejects malformed scenario names before process execution", () => {
  assert.throws(() => client.runScenario("../baseline"), TypeError);
});

test("SDK reports a missing executable with structured context", () => {
  const missing = new GraniteClient({ binary: join(tmpdir(), "missing-granitedtl") });
  assert.throws(
    () => missing.runScenario("baseline"),
    (error) => error instanceof GraniteExecutionError && typeof error.details.binary === "string",
  );
});

test("report validation fails closed on incomplete payloads", () => {
  assert.throws(() => validateReport({ scenario: "partial" }), /missing policy/);
});
