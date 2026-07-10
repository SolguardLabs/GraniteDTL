import assert from "node:assert/strict";
import { existsSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { spawnSync } from "node:child_process";

const root = resolve(dirname(fileURLToPath(import.meta.url)), "..", "..");
const exeName = process.platform === "win32" ? "granitedtl.exe" : "granitedtl";
const exePath = join(root, "build", exeName);

let built = false;

export function ensureBuilt() {
  if (built && existsSync(exePath)) {
    return exePath;
  }

  const result = spawnSync(process.execPath, ["scripts/build.mjs"], {
    cwd: root,
    encoding: "utf8",
    stdio: "pipe",
  });

  assert.equal(result.status, 0, result.stderr || result.stdout);
  assert.equal(existsSync(exePath), true, "expected GraniteDTL binary to exist");
  built = true;
  return exePath;
}

export function runRaw(args = []) {
  const binary = ensureBuilt();
  const result = spawnSync(binary, args, {
    cwd: root,
    encoding: "utf8",
    stdio: "pipe",
  });

  assert.equal(result.status, 0, result.stderr || result.stdout);
  return result.stdout;
}

export function runJson(args = []) {
  const stdout = runRaw(args);
  return JSON.parse(stdout);
}

export { root, exePath };
