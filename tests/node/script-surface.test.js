import test from "node:test";
import assert from "node:assert/strict";
import { mkdtempSync, writeFileSync } from "node:fs";
import { join } from "node:path";
import { tmpdir } from "node:os";

import { runJson } from "../helpers/granite-cli.js";

test("script mode executes a custom healthy flow", () => {
  const dir = mkdtempSync(join(tmpdir(), "granite-script-"));
  const script = join(dir, "healthy.gdtl");
  writeFileSync(
    script,
    [
      "SCENARIO custom-healthy",
      "EMPTY",
      "ACCOUNT alice Alice 2200000",
      "ACCOUNT merchant Merchant 50000",
      "RESERVE 700000 seed",
      "OPEN p1 alice merchant 1700000 1000000 5 500 lane-a",
      "REFRESH p1",
      "RELEASE p1",
      "TRY_COMPLETE missing",
      "ADVANCE 2",
      "COMPLETE p1",
      "",
    ].join("\n"),
  );

  const report = runJson(["script", script]);
  assert.equal(report.scenario, "custom-healthy");
  assert.equal(report.positions.p1.state, "closed");
  assert.equal(report.positions.p1.terminal, true);
  assert.equal(report.journal.rejected_events, 1);
  assert.equal(report.checks.accounting_ok, true);
});

test("script mode supports policy updates and settlement", () => {
  const dir = mkdtempSync(join(tmpdir(), "granite-script-"));
  const script = join(dir, "settlement.gdtl");
  writeFileSync(
    script,
    [
      "SCENARIO custom-settlement",
      "EMPTY",
      "POLICY daily_penalty_bps 200",
      "ACCOUNT alice Alice 2100000",
      "ACCOUNT merchant Merchant 50000",
      "RESERVE 700000 seed",
      "OPEN p2 alice merchant 1500000 1000000 1 1000 lane-b",
      "ADVANCE 5",
      "SETTLE p2",
      "",
    ].join("\n"),
  );

  const report = runJson(["script", script]);
  assert.equal(report.scenario, "custom-settlement");
  assert.equal(report.positions.p2.state, "defaulted");
  assert.ok(report.positions.p2.penalty_accrued > 0);
  assert.ok(report.positions.p2.shortfall > 0);
});
