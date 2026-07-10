import { existsSync, mkdirSync, readdirSync, writeFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { spawnSync } from "node:child_process";

const root = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const srcDir = join(root, "src");
const buildDir = join(root, "build");
const exeName = process.platform === "win32" ? "granitedtl.exe" : "granitedtl";
const exePath = join(buildDir, exeName);
const sources = readdirSync(srcDir)
  .filter((name) => name.endsWith(".cpp"))
  .sort()
  .map((name) => join(srcDir, name));

mkdirSync(buildDir, { recursive: true });

function run(command, args, options = {}) {
  const result = spawnSync(command, args, {
    cwd: root,
    encoding: "utf8",
    stdio: options.capture ? "pipe" : "inherit",
    shell: options.shell ?? false,
  });

  if (options.capture) {
    return result;
  }

  if (result.status !== 0) {
    process.exit(result.status ?? 1);
  }

  return result;
}

function findOnPath(name) {
  const result =
    process.platform === "win32"
      ? run("where.exe", [name], { capture: true })
      : run("command", ["-v", name], { capture: true });

  if (result.status !== 0) {
    return null;
  }

  return result.stdout.split(/\r?\n/).find(Boolean)?.trim() ?? null;
}

function quoteCmd(value) {
  const text = String(value);
  if (!/[ \t&()]/.test(text)) {
    return text;
  }
  return `"${text.replace(/"/g, '""')}"`;
}

function buildWithUnixCompiler(compiler) {
  run(compiler, [
    "-std=c++17",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-O2",
    "-I",
    srcDir,
    "-o",
    exePath,
    ...sources,
  ]);
}

function findVsInstallPath() {
  const candidates = [
    "C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe",
    "C:\\Program Files\\Microsoft Visual Studio\\Installer\\vswhere.exe",
  ];
  const vswhere = candidates.find((candidate) => existsSync(candidate));
  if (!vswhere) {
    return null;
  }

  const result = run(
    vswhere,
    [
      "-latest",
      "-products",
      "*",
      "-requires",
      "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
      "-property",
      "installationPath",
    ],
    { capture: true },
  );

  if (result.status !== 0) {
    return null;
  }

  return result.stdout.split(/\r?\n/).find(Boolean)?.trim() ?? null;
}

function findVcvars() {
  const installPath = findVsInstallPath();
  const candidates = [
    installPath ? join(installPath, "VC", "Auxiliary", "Build", "vcvars64.bat") : null,
    "C:\\Program Files\\Microsoft Visual Studio\\18\\Insiders\\VC\\Auxiliary\\Build\\vcvars64.bat",
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat",
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Auxiliary\\Build\\vcvars64.bat",
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Auxiliary\\Build\\vcvars64.bat",
    "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat",
  ].filter(Boolean);

  return candidates.find((candidate) => existsSync(candidate)) ?? null;
}

function buildWithMsvc() {
  const clOnPath = findOnPath("cl.exe");
  const vcvars = findVcvars();
  const sourceArgs = sources.map((source) => join("src", source.split(/[\\/]/).pop()));
  const commonArgs = [
    "/nologo",
    "/std:c++17",
    "/EHsc",
    "/W4",
    "/WX",
    "/O2",
    "/I",
    "src",
    `/Fe:build\\${exeName}`,
    "/Fobuild\\",
    ...sourceArgs,
  ];

  if (clOnPath) {
    run("cl.exe", commonArgs);
    return;
  }

  if (!vcvars) {
    console.error("No C++ compiler found. Install GCC/Clang or Visual Studio Build Tools.");
    process.exit(127);
  }

  const cmdPath = join(buildDir, "build-msvc.cmd");
  const lines = [
    "@echo off",
    `call ${quoteCmd(vcvars)} x64`,
    "if errorlevel 1 exit /b %errorlevel%",
    `cl.exe ${commonArgs.map(quoteCmd).join(" ")}`,
    "exit /b %errorlevel%",
    "",
  ];
  writeFileSync(cmdPath, lines.join("\r\n"), "utf8");
  run("cmd.exe", ["/d", "/c", cmdPath]);
}

const explicit = process.env.CXX;
if (explicit) {
  buildWithUnixCompiler(explicit);
} else if (process.platform === "win32") {
  const pathCompiler = findOnPath("c++") || findOnPath("g++") || findOnPath("clang++");
  if (pathCompiler) {
    buildWithUnixCompiler(pathCompiler);
  } else {
    buildWithMsvc();
  }
} else {
  const compiler = findOnPath("c++") || findOnPath("g++") || findOnPath("clang++");
  if (!compiler) {
    console.error("No C++ compiler found. Install c++, g++ or clang++.");
    process.exit(127);
  }
  buildWithUnixCompiler(compiler);
}

console.log(exePath);
