#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
BINARY=${BINARY:-"${ROOT_DIR}/secret"}
TMP_ROOT="${ROOT_DIR}/tests/_tmp"

mkdir -p "${TMP_ROOT}"

PASS=0
FAIL=0

log_info()  { echo "[INFO] $*"; }
log_pass()  { echo "[PASS] $*"; ((PASS++)) || true; }
log_fail()  { echo "[FAIL] $*"; ((FAIL++)) || true; }

clean_test_dir() {
    local name=$1
    rm -rf "${TMP_ROOT}/${name}"
    mkdir -p "${TMP_ROOT}/${name}/client" "${TMP_ROOT}/${name}/server"
}

make_random_file() {
    local path=$1
    local size=$2
    dd if=/dev/urandom of="${path}" bs=1 count="${size}" status=none
}

wait_for_pid() {
    local pid=$1
    local timeout=$2
    local start
    start=$(date +%s)
    while kill -0 "${pid}" 2>/dev/null; do
        local now
        now=$(date +%s)
        if (( now - start >= timeout )); then
            return 1
        fi
        sleep 0.1
    done
    return 0
}

run_transfer_test() {
    local name=$1
    local size=$2
    local ip=$3

    clean_test_dir "${name}"
    local test_dir="${TMP_ROOT}/${name}"
    local client_dir="${test_dir}/client"
    local server_dir="${test_dir}/server"
    local src_file="input.bin"

    log_info "Running test ${name} (size ${size}, ${ip})"

    make_random_file "${client_dir}/${src_file}" "${size}"

    (
        cd "${server_dir}"
        "${BINARY}" -l >"${test_dir}/server.log" 2>&1
    ) &
    local server_pid=$!

    sleep 0.3

    local client_rc=0
    (
        cd "${client_dir}"
        "${BINARY}" -r "${src_file}" -s "${ip}" >"${test_dir}/client.log" 2>&1
    ) || client_rc=$?

    if kill -INT "${server_pid}" 2>/dev/null; then
        wait_for_pid "${server_pid}" 5 || kill -KILL "${server_pid}" 2>/dev/null || true
    fi

    if [[ ${client_rc} -ne 0 ]]; then
        log_fail "${name}: client exited with code ${client_rc}"
        return
    fi

    local out_file="${server_dir}/${src_file}"
    if [[ ! -f "${out_file}" ]]; then
        log_fail "${name}: server did not produce file ${out_file}"
        return
    fi

    if cmp -s "${client_dir}/${src_file}" "${out_file}"; then
        log_pass "${name}"
    else
        log_fail "${name}: input and output differ"
    fi
}

test_help() {
    log_info "Running test help"
    local out rc=0
    out=$("${BINARY}" -h 2>&1) || rc=$?
    if [[ ${rc} -ne 0 ]]; then
        log_fail "help: expected rc=0, got ${rc}"
        return
    fi
    if grep -q "Usage: secret -r <file> -s <ip|hostname> |  -l" <<<"${out}"; then
        log_pass "help"
    else
        log_fail "help: usage line not found in output"
    fi
}

test_no_args() {
    log_info "Running test no_args"
    local out rc=0
    out=$("${BINARY}" 2>&1) || rc=$?
    if [[ ${rc} -ne 1 ]]; then
        log_fail "no_args: expected rc=1, got ${rc}"
        return
    fi
    if grep -q "Error: Any argument was not provided!" <<<"${out}"; then
        log_pass "no_args"
    else
        log_fail "no_args: expected error message not found"
    fi
}

test_invalid_flag() {
    log_info "Running test invalid_flag"
    local out rc=0
    out=$("${BINARY}" -x 2>&1) || rc=$?
    if [[ ${rc} -ne 1 ]]; then
        log_fail "invalid_flag: expected rc=1, got ${rc}"
        return
    fi
    if grep -q "Error: Invalid argument or argument was provided multiple times!" <<<"${out}"; then
        log_pass "invalid_flag"
    else
        log_fail "invalid_flag: expected error message not found"
    fi
}

test_missing_value_for_r() {
    log_info "Running test missing_value_for_r"
    local out rc=0
    rc=0
    out=$("${BINARY}" -r 2>&1) || rc=$?
    if [[ ${rc} -eq 0 ]]; then
        log_fail "missing_value_for_r: program unexpectedly exited with rc=0"
        return
    fi
    log_pass "missing_value_for_r (only rc!=0 checked)"
}

test_missing_file() {
    log_info "Running test missing_file"
    local tmp_dir="${TMP_ROOT}/missing_file"
    mkdir -p "${tmp_dir}"
    local out rc=0
    out=$(
        cd "${tmp_dir}"
        "${BINARY}" -r not_exists.bin -s 127.0.0.1 2>&1
    ) || rc=$?
    if [[ ${rc} -ne 1 ]]; then
        log_fail "missing_file: expected rc=1, got ${rc}"
        return
    fi
    if grep -q "Error: Provided file does not exist!" <<<"${out}"; then
        log_pass "missing_file"
    else
        log_fail "missing_file: expected error message not found"
    fi
}

test_r_is_directory() {
    log_info "Running test r_is_directory"
    local tmp_dir="${TMP_ROOT}/r_is_directory"
    mkdir -p "${tmp_dir}/as_dir"
    local out rc=0
    out=$(
        cd "${tmp_dir}"
        "${BINARY}" -r as_dir -s 127.0.0.1 2>&1
    ) || rc=$?
    if [[ ${rc} -ne 1 ]]; then
        log_fail "r_is_directory: expected rc=1, got ${rc}"
        return
    fi
    if grep -q "Error: Provided file is not a file!" <<<"${out}"; then
        log_pass "r_is_directory"
    else
        log_fail "r_is_directory: expected error message not found"
    fi
}

test_ipv4_small()      { run_transfer_test "ipv4_small"      512     "127.0.0.1"; }
test_ipv4_large()      { run_transfer_test "ipv4_large"      250000  "127.0.0.1"; }
test_ipv4_edge_1B()    { run_transfer_test "ipv4_edge_1B"    1       "127.0.0.1"; }
test_ipv4_edge_chunk() { run_transfer_test "ipv4_edge_chunk" 1392    "127.0.0.1"; }

test_ipv6_small()      { run_transfer_test "ipv6_small"      4096    "::1"; }
test_ipv6_large()      { run_transfer_test "ipv6_large"      520000  "::1"; }
test_ipv6_edge_1B()    { run_transfer_test "ipv6_edge_1B"    1       "::1"; }
test_ipv6_edge_chunk() { run_transfer_test "ipv6_edge_chunk" 1392    "::1"; }

ALL_TESTS=(
    test_help
    test_no_args
    test_invalid_flag
    test_missing_value_for_r
    test_missing_file
    test_r_is_directory
    test_ipv4_small
    test_ipv4_large
    test_ipv4_edge_1B
    test_ipv4_edge_chunk
    test_ipv6_small
    test_ipv6_large
    test_ipv6_edge_1B
    test_ipv6_edge_chunk
)

main() {
    if [[ ${EUID:-$(id -u)} -ne 0 ]]; then
        echo "This test suite must be run as root (raw ICMP sockets)." >&2
        exit 1
    fi
    if [[ ! -x "${BINARY}" ]]; then
        echo "Binary '${BINARY}' not found or not executable." >&2
        exit 1
    fi

    log_info "Running ${#ALL_TESTS[@]} tests for ${BINARY}"
    for t in "${ALL_TESTS[@]}"; do
        if declare -F "${t}" >/dev/null 2>&1; then
            ${t}
        else
            log_fail "Test function ${t} not defined"
        fi
    done

    echo
    echo "Summary: PASS=${PASS}, FAIL=${FAIL}"
    [[ ${FAIL} -eq 0 ]]
}

main "$@"

