#!/bin/bash
# ──────────────────────────────────────────────────────────────────
#  test_mysh.sh — Automated test suite for mysh (Mini Unix Shell)
#
#  Usage:  chmod +x tests/test_mysh.sh && ./tests/test_mysh.sh
#
#  Returns exit code 0 if all tests pass, 1 otherwise.
# ──────────────────────────────────────────────────────────────────

set -e

SHELL_BIN="./bin/mysh"
PASS=0
FAIL=0
TOTAL=0

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

# ── Helper: run a command in mysh and capture output ──────────────
run_test() {
    local description="$1"
    local input="$2"
    local expected="$3"

    TOTAL=$((TOTAL + 1))

    # Run the shell with the input, capture stdout only
    actual=$(echo "$input" | $SHELL_BIN 2>/dev/null)

    if echo "$actual" | grep -qF "$expected"; then
        PASS=$((PASS + 1))
        echo -e "  ${GREEN}✓${NC} ${description}"
    else
        FAIL=$((FAIL + 1))
        echo -e "  ${RED}✗${NC} ${description}"
        echo -e "    Expected to contain: ${CYAN}${expected}${NC}"
        echo -e "    Got: ${CYAN}${actual}${NC}"
    fi
}

# ── Helper: test that output file was created ─────────────────────
run_file_test() {
    local description="$1"
    local input="$2"
    local file="$3"
    local expected_content="$4"

    TOTAL=$((TOTAL + 1))

    # Clean up first
    rm -f "$file"

    echo "$input" | $SHELL_BIN 2>/dev/null

    if [ -f "$file" ] && grep -qF "$expected_content" "$file"; then
        PASS=$((PASS + 1))
        echo -e "  ${GREEN}✓${NC} ${description}"
    else
        FAIL=$((FAIL + 1))
        echo -e "  ${RED}✗${NC} ${description}"
        if [ -f "$file" ]; then
            echo -e "    File content: ${CYAN}$(cat "$file")${NC}"
        else
            echo -e "    File ${CYAN}${file}${NC} was not created"
        fi
    fi

    rm -f "$file"
}

# ── Build first ───────────────────────────────────────────────────
echo -e "\n${BOLD}╔══════════════════════════════════════╗${NC}"
echo -e "${BOLD}║     mysh — Automated Test Suite      ║${NC}"
echo -e "${BOLD}╚══════════════════════════════════════╝${NC}\n"

echo -e "${BOLD}[1/8] Building mysh...${NC}"
if make clean > /dev/null 2>&1 && make > /dev/null 2>&1; then
    echo -e "  ${GREEN}✓${NC} Build successful"
else
    echo -e "  ${RED}✗${NC} Build FAILED"
    exit 1
fi

if [ ! -x "$SHELL_BIN" ]; then
    echo -e "  ${RED}✗${NC} Binary not found at $SHELL_BIN"
    exit 1
fi

# ══════════════════════════════════════════════════════════════════
# TEST SUITE
# ══════════════════════════════════════════════════════════════════

# ── Test Group 1: Basic Command Execution ─────────────────────────
echo -e "\n${BOLD}[2/8] Basic Command Execution${NC}"
run_test "echo command" \
    "echo hello world" \
    "hello world"

run_test "date command runs" \
    "date +%Y" \
    "$(date +%Y)"

run_test "whoami command" \
    "whoami" \
    "$(whoami)"

run_test "uname command" \
    "uname" \
    "Linux"

# ── Test Group 2: Built-in: pwd ───────────────────────────────────
echo -e "\n${BOLD}[3/8] Built-in Commands: pwd${NC}"
run_test "pwd prints current directory" \
    "pwd" \
    "$(pwd)"

# ── Test Group 3: Built-in: cd ────────────────────────────────────
echo -e "\n${BOLD}[4/8] Built-in Commands: cd${NC}"
run_test "cd /tmp then pwd" \
    "cd /tmp
pwd" \
    "/tmp"

run_test "cd ~ then pwd shows HOME" \
    "cd ~
pwd" \
    "$HOME"

run_test "cd (no args) goes HOME" \
    "cd
pwd" \
    "$HOME"

# ── Test Group 4: Piping ──────────────────────────────────────────
echo -e "\n${BOLD}[5/8] Piping${NC}"
run_test "echo | cat" \
    "echo piping_works | cat" \
    "piping_works"

run_test "echo | grep" \
    "echo hello_world | grep hello" \
    "hello_world"

run_test "echo | wc -w (word count)" \
    "echo one two three | wc -w" \
    "3"

run_test "Three-stage pipeline" \
    'echo -e "cherry\napple\nbanana" | sort | head -1' \
    "apple"

# ── Test Group 5: Output Redirection ──────────────────────────────
echo -e "\n${BOLD}[6/8] I/O Redirection${NC}"
run_file_test "Output redirect >" \
    "echo redirect_test > /tmp/mysh_test_out.txt" \
    "/tmp/mysh_test_out.txt" \
    "redirect_test"

run_file_test "Append redirect >>" \
    'echo line1 > /tmp/mysh_test_append.txt
echo line2 >> /tmp/mysh_test_append.txt' \
    "/tmp/mysh_test_append.txt" \
    "line2"

# Input redirection
TOTAL=$((TOTAL + 1))
echo "input_redir_content" > /tmp/mysh_test_input.txt
actual=$(echo "cat < /tmp/mysh_test_input.txt" | $SHELL_BIN 2>/dev/null)
if echo "$actual" | grep -qF "input_redir_content"; then
    PASS=$((PASS + 1))
    echo -e "  ${GREEN}✓${NC} Input redirect <"
else
    FAIL=$((FAIL + 1))
    echo -e "  ${RED}✗${NC} Input redirect <"
fi
rm -f /tmp/mysh_test_input.txt

# ── Test Group 6: History ─────────────────────────────────────────
echo -e "\n${BOLD}[7/8] Command History${NC}"
run_test "history records commands" \
    "echo first
echo second
history 2" \
    "echo second"

run_test "history shows count" \
    "echo a
echo b
echo c
history 3" \
    "echo a"

# ── Test Group 7: Edge Cases ──────────────────────────────────────
echo -e "\n${BOLD}[8/8] Edge Cases${NC}"

# Empty input shouldn't crash
TOTAL=$((TOTAL + 1))
result=$(printf "\n\n\nexit\n" | $SHELL_BIN 2>/dev/null; echo "OK")
if echo "$result" | grep -qF "OK"; then
    PASS=$((PASS + 1))
    echo -e "  ${GREEN}✓${NC} Empty input handled gracefully"
else
    FAIL=$((FAIL + 1))
    echo -e "  ${RED}✗${NC} Empty input handling"
fi

# Invalid command
TOTAL=$((TOTAL + 1))
result=$(echo "nonexistent_cmd_xyz" | $SHELL_BIN 2>&1)
if echo "$result" | grep -qi "nonexistent_cmd_xyz"; then
    PASS=$((PASS + 1))
    echo -e "  ${GREEN}✓${NC} Invalid command shows error"
else
    FAIL=$((FAIL + 1))
    echo -e "  ${RED}✗${NC} Invalid command error message"
fi

# Quoted strings
run_test "Double-quoted string" \
    'echo "hello   world"' \
    "hello   world"

# ══════════════════════════════════════════════════════════════════
# RESULTS
# ══════════════════════════════════════════════════════════════════
echo -e "\n${BOLD}════════════════════════════════════════${NC}"
echo -e "${BOLD}  Results: ${GREEN}${PASS} passed${NC}, ${RED}${FAIL} failed${NC}, ${BOLD}${TOTAL} total${NC}"
echo -e "${BOLD}════════════════════════════════════════${NC}\n"

# Cleanup
rm -f /tmp/mysh_test_*.txt

if [ "$FAIL" -eq 0 ]; then
    echo -e "${GREEN}${BOLD}  ✓ All tests passed! Shell is working correctly.${NC}\n"
    exit 0
else
    echo -e "${RED}${BOLD}  ✗ Some tests failed. Review output above.${NC}\n"
    exit 1
fi
