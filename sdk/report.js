const requiredSections = Object.freeze([
  "policy",
  "vault",
  "accounts",
  "positions",
  "events",
  "checks",
  "invariants",
  "treasury_stress",
]);

function assertRecord(value, field) {
  if (value === null || typeof value !== "object" || Array.isArray(value)) {
    throw new TypeError(`${field} must be an object`);
  }
}

export function validateReport(report) {
  assertRecord(report, "report");
  for (const section of requiredSections) {
    if (!(section in report)) {
      throw new TypeError(`report is missing ${section}`);
    }
  }

  if (typeof report.scenario !== "string" || report.scenario.length === 0) {
    throw new TypeError("report scenario must be a non-empty string");
  }
  if (!Array.isArray(report.events)) {
    throw new TypeError("report events must be an array");
  }
  if (!Array.isArray(report.treasury_stress.scenarios)) {
    throw new TypeError("treasury stress scenarios must be an array");
  }

  return deepFreeze(report);
}

export function deepFreeze(value) {
  if (value === null || typeof value !== "object" || Object.isFrozen(value)) {
    return value;
  }
  for (const child of Object.values(value)) {
    deepFreeze(child);
  }
  return Object.freeze(value);
}
