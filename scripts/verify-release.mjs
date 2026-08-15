import assert from "node:assert/strict";
import { createHash } from "node:crypto";
import { execFileSync } from "node:child_process";
import { existsSync, readFileSync } from "node:fs";
import { extname, resolve } from "node:path";

const root = resolve(import.meta.dirname, "..");
const expectedDocs = [
  "docs/arquitectura.md",
  "docs/ciclo-garantias.md",
  "docs/despliegue.md",
  "docs/integracion.md",
  "docs/modelo-economico.md",
  "docs/operaciones.md",
  "docs/seguridad-operativa.md",
];
const protectedFiles = new Map([
  ["src/engine.cpp", "EE6462A4421BEF4DE3F1AAA8092AC3E7E6B94484CAA1A2C382390E6EDD47CA98"],
  ["src/risk.cpp", "914196DDBCB7EF2BADFD3CE56A2FF71C0FC13F066357C0D3BD376555B1DCCBB6"],
  ["src/locks.cpp", "8B9DC9B5269C46F7EE6EC440D51E6956D5C20CFDC6D9E61F4D61016E52CDBA45"],
]);
const textExtensions = new Set([
  ".cpp",
  ".hpp",
  ".js",
  ".mjs",
  ".json",
  ".md",
  ".yml",
  ".yaml",
  ".sh",
  ".gdtl",
  ".txt",
]);

function sha256(buffer) {
  return createHash("sha256").update(buffer).digest("hex").toUpperCase();
}

function normalizedFile(file) {
  const value = readFileSync(resolve(root, file), "utf8").replace(/\r\n/g, "\n");
  return Buffer.from(value, "utf8");
}

function repositoryFiles() {
  return execFileSync("git", ["ls-files", "--cached", "--others", "--exclude-standard", "-z"], {
    cwd: root,
  })
    .toString("utf8")
    .split("\0")
    .filter(Boolean)
    .filter((file) => existsSync(resolve(root, file)))
    .sort();
}

const files = repositoryFiles();
const packageJson = JSON.parse(readFileSync(resolve(root, "package.json"), "utf8"));
const packageLock = JSON.parse(readFileSync(resolve(root, "package-lock.json"), "utf8"));

assert.equal(packageJson.version, "1.0.0");
assert.equal(packageLock.version, "1.0.0");
assert.equal(packageLock.packages[""].version, "1.0.0");
assert.deepEqual(
  files.filter((file) => file.startsWith("docs/") && file.endsWith(".md")),
  expectedDocs,
);

for (const [file, expectedHash] of protectedFiles) {
  assert.equal(sha256(normalizedFile(file)), expectedHash, `${file} changed`);
}

const banner = readFileSync(resolve(root, "assets/banner.png"));
assert.equal(sha256(banner), "EEAE5EB227BC44CF72726C847FB1AD8C56B5C6946157B3C8723153BB308EF1C7");
assert.equal(banner.readUInt32BE(16), 1672);
assert.equal(banner.readUInt32BE(20), 941);

const documentation = ["README.md", "SECURITY.md", ...expectedDocs]
  .map((file) => readFileSync(resolve(root, file), "utf8"))
  .join("\n");
assert.equal((documentation.match(/^```mermaid$/gm) ?? []).length, 27);
assert.match(readFileSync(resolve(root, "README.md"), "utf8"), /assets\/banner\.png/);

const restrictedTerms = [
  "c" + "tf",
  "la" + "bs?",
  "labor" + "atorios?",
  "vulner" + "ability",
  "vulner" + "abilidad",
  "vulner" + "able",
  "bu" + "gs?",
  "ex" + "ploit",
  "by" + "pass",
  "attack" + "ers?",
  "atac" + "antes?",
];
const forbidden = new RegExp(`\\b(?:${restrictedTerms.join("|")})\\b`, "i");
for (const file of files) {
  if (!textExtensions.has(extname(file).toLowerCase())) {
    continue;
  }
  const contents = readFileSync(resolve(root, file), "utf8");
  assert.equal(forbidden.test(contents), false, `restricted public term in ${file}`);
}

const sourceLines = files
  .filter((file) => file.endsWith(".cpp") || file.endsWith(".hpp"))
  .flatMap((file) => readFileSync(resolve(root, file), "utf8").split(/\r?\n/))
  .filter((line) => line.trim().length > 0).length;
const nodeTests = files
  .filter((file) => file.startsWith("tests/node/") && file.endsWith(".test.js"))
  .map((file) => readFileSync(resolve(root, file), "utf8"))
  .join("\n")
  .match(/\btest\s*\(/g)?.length;

assert.ok(sourceLines >= 3_000, `expected at least 3000 C++ source lines, got ${sourceLines}`);
assert.ok((nodeTests ?? 0) >= 19, `expected at least 19 Node tests, got ${nodeTests ?? 0}`);
assert.ok(files.includes("sdk/client.js"));
assert.ok(files.includes("sdk/report.js"));

for (const workflow of [".github/workflows/ci.yml", ".github/workflows/release-integrity.yml"]) {
  const contents = readFileSync(resolve(root, workflow), "utf8");
  assert.match(contents, /actions\/checkout@v7/);
  assert.match(contents, /actions\/setup-node@v7/);
}

process.stdout.write(
  `${JSON.stringify({
    version: packageJson.version,
    docs: expectedDocs.length,
    diagrams: 27,
    cpp_source_lines: sourceLines,
    node_tests: nodeTests,
    banner_sha256: sha256(banner),
  })}\n`,
);
