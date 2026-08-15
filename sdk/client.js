import { existsSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { spawnSync } from "node:child_process";

import { validateReport } from "./report.js";

const repositoryRoot = resolve(dirname(fileURLToPath(import.meta.url)), "..");

export class GraniteExecutionError extends Error {
  constructor(message, details = {}) {
    super(message);
    this.name = "GraniteExecutionError";
    this.details = Object.freeze({ ...details });
  }
}

export class GraniteClient {
  constructor(options = {}) {
    const executable = process.platform === "win32" ? "granitedtl.exe" : "granitedtl";
    this.root = resolve(options.root ?? repositoryRoot);
    this.binary = resolve(options.binary ?? join(this.root, "build", executable));
    this.timeoutMs = options.timeoutMs ?? 15_000;
    this.maxBuffer = options.maxBuffer ?? 8 * 1024 * 1024;
  }

  runScenario(name) {
    if (typeof name !== "string" || !/^[a-z][a-z0-9-]{0,63}$/.test(name)) {
      throw new TypeError("scenario name must use lower-case letters, digits or hyphens");
    }
    return this.#execute([name]);
  }

  runScript(scriptPath) {
    const resolved = resolve(scriptPath);
    if (!existsSync(resolved)) {
      throw new GraniteExecutionError("scenario file does not exist", {
        script: resolved,
      });
    }
    return this.#execute(["script", resolved]);
  }

  listScenarios() {
    const report = this.#executeRaw(["--list"]);
    if (!Array.isArray(report.scenarios)) {
      throw new GraniteExecutionError("scenario registry returned an invalid payload");
    }
    return Object.freeze([...report.scenarios]);
  }

  #execute(args) {
    return validateReport(this.#executeRaw(args));
  }

  #executeRaw(args) {
    if (!existsSync(this.binary)) {
      throw new GraniteExecutionError("GraniteDTL binary is not available", {
        binary: this.binary,
      });
    }

    const result = spawnSync(this.binary, args, {
      cwd: this.root,
      encoding: "utf8",
      shell: false,
      timeout: this.timeoutMs,
      maxBuffer: this.maxBuffer,
      windowsHide: true,
    });

    if (result.error) {
      throw new GraniteExecutionError("protocol process could not be executed", {
        cause: result.error.message,
      });
    }
    if (result.status !== 0) {
      throw new GraniteExecutionError("protocol process rejected the request", {
        status: result.status,
        stderr: result.stderr.trim(),
      });
    }

    try {
      return JSON.parse(result.stdout);
    } catch (error) {
      throw new GraniteExecutionError("protocol process returned invalid JSON", {
        cause: error.message,
      });
    }
  }
}
