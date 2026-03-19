#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

g++ -std=c++17 -O3 -pthread \
  "${SCRIPT_DIR}/neurone_simulator.cpp" \
  -o "${SCRIPT_DIR}/neurone_simulator"
