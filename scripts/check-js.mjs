import { readdirSync, statSync } from "node:fs";
import { join, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { spawnSync } from "node:child_process";

const root = resolve(fileURLToPath(new URL("..", import.meta.url)));
const dirs = ["scripts", "tests"];

function walk(dir) {
  const result = [];
  for (const entry of readdirSync(dir)) {
    const full = join(dir, entry);
    const stat = statSync(full);
    if (stat.isDirectory()) {
      result.push(...walk(full));
    } else if (entry.endsWith(".js") || entry.endsWith(".mjs")) {
      result.push(full);
    }
  }
  return result;
}

for (const dir of dirs) {
  for (const file of walk(join(root, dir))) {
    const result = spawnSync(process.execPath, ["--check", file], {
      cwd: root,
      stdio: "inherit",
    });
    if (result.status !== 0) {
      process.exit(result.status ?? 1);
    }
  }
}
