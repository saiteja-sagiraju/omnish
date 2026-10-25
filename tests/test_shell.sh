#!/usr/bin/env bash
set -eu

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SHELL_BIN="$ROOT/compiled/omnish"
TMP_DIR="$(mktemp -d)"

cleanup() {
    rm -rf "$TMP_DIR"
}
trap cleanup EXIT

make -C "$ROOT" >/dev/null

run_shell() {
    OMNISH_COMMAND="$1" SHELL_BIN="$SHELL_BIN" python3 - <<'PY'
import os
import signal
import subprocess
import sys
import time

process = subprocess.Popen(
    ["script", "-qefc", os.environ["SHELL_BIN"], "/dev/null"],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
)

time.sleep(0.2)
output = bytearray()
process.stdin.write(os.environ["OMNISH_COMMAND"].encode() + b"\n")
process.stdin.flush()
time.sleep(1)

process.send_signal(signal.SIGTERM)

try:
    process.wait(timeout=5)
except subprocess.TimeoutExpired:
    process.kill()
    output = process.stdout.read()
    sys.stderr.buffer.write(output)
    raise SystemExit("timed out waiting for shell to exit")

output.extend(process.stdout.read())
if process.returncode not in (0, 130):
    sys.stderr.buffer.write(output)
    raise SystemExit(1)

sys.stdout.buffer.write(output)
PY
}

assert_contains() {
    local output="$1"
    local expected="$2"

    if ! grep -Fq "$expected" <<< "$output"; then
        echo "FAIL: expected output to contain: $expected"
        echo "Actual output:"
        echo "$output"
        exit 1
    fi
}

echo "Testing basic command..."
output="$(run_shell 'echo hello')"
assert_contains "$output" "hello"

echo "Testing arguments..."
output="$(run_shell 'echo one two three')"
assert_contains "$output" "one two three"

echo "Testing quotes..."
output="$(run_shell 'echo "hello world"')"
assert_contains "$output" "hello world"

echo "Testing pipe..."
output="$(run_shell 'printf "apple\nbanana\n" | grep banana')"
assert_contains "$output" "banana"

echo "Testing output redirection..."
output_file="$TMP_DIR/output.txt"
run_shell "echo redirected > $output_file" >/dev/null

if [ "$(cat "$output_file")" != "redirected" ]; then
    echo "FAIL: output redirection"
    exit 1
fi

echo "Testing append redirection..."
run_shell "echo second >> $output_file" >/dev/null
assert_contains "$(cat "$output_file")" "second"

echo "Testing input redirection..."
input_file="$TMP_DIR/input.txt"
printf 'line one\nline two\n' > "$input_file"

output="$(run_shell "wc -l < $input_file")"
assert_contains "$output" "2"

echo "Testing AND..."
output="$(run_shell 'true && echo success')"
assert_contains "$output" "success"

output="$(run_shell 'false && echo should-not-run')"
if grep -Fxq "should-not-run" <<< "$output"; then
    echo "FAIL: false && command ran"
    exit 1
fi

echo "Testing OR..."
output="$(run_shell 'false || echo fallback')"
assert_contains "$output" "fallback"

output="$(run_shell 'true || echo should-not-run')"
if grep -Fxq "should-not-run" <<< "$output"; then
    echo "FAIL: true || command ran"
    exit 1
fi

echo "Testing EOF handling..."
output="$(run_shell 'echo eof-test')"
assert_contains "$output" "eof-test"

echo "All tests passed."