#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${root}"

node scripts/check-js.mjs
node scripts/build.mjs
node --test --test-concurrency=1 "tests/node/*.test.js"
