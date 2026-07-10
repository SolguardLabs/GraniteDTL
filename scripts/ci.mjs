import { spawnSync } from "node:child_process";
import { readdirSync } from "node:fs";
import { join } from "node:path";

function run(command, args) {
  const result = spawnSync(command, args, {
    stdio: "inherit",
    shell: false,
  });

  if (result.status !== 0) {
    process.exit(result.status ?? 1);
  }
}

const testFiles = readdirSync(join("tests", "node"))
  .filter((name) => name.endsWith(".test.js"))
  .sort()
  .map((name) => join("tests", "node", name));

run(process.execPath, ["scripts/check-js.mjs"]);
run(process.execPath, ["scripts/build.mjs"]);
run(process.execPath, ["--test", "--test-concurrency=1", ...testFiles]);
