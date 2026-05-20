#!/usr/bin/env bash

OUT_DIR="output"
SUMMARY_FILE="${OUT_DIR}/summary.csv"

mkdir -p "${OUT_DIR}"

echo "case,ast,dot,png,ir,symbols,semantic_errors,warnings,status" > "${SUMMARY_FILE}"

for src in examples/*.src; do
    base=$(basename "$src" .src)

    ast_file="${OUT_DIR}/${base}.ast.txt"
    dot_file="${OUT_DIR}/${base}.ast.dot"
    png_file="${OUT_DIR}/${base}.ast.png"
    ir_file="${OUT_DIR}/${base}.ir.txt"
    sym_file="${OUT_DIR}/${base}.symbols.txt"
    err_file="${OUT_DIR}/${base}.errors.txt"

    ast_ok="no"
    dot_ok="no"
    png_ok="no"
    ir_ok="no"
    sym_ok="no"
    error_count=0
    warning_count=0
    status="PASS"

    if [ -s "$ast_file" ]; then ast_ok="yes"; fi
    if [ -s "$dot_file" ]; then dot_ok="yes"; fi
    if [ -s "$png_file" ]; then png_ok="yes"; fi
    if [ -s "$ir_file" ]; then ir_ok="yes"; fi
    if [ -s "$sym_file" ]; then sym_ok="yes"; fi

    if [ -f "$err_file" ]; then
        error_count=$(awk '
            BEGIN { inerr=0; c=0 }
            /^Semantic errors:/ { inerr=1; next }
            /^Warnings:/ { inerr=0; next }
            inerr && /^- / { c++ }
            END { print c }
        ' "$err_file")

        warning_count=$(awk '
            BEGIN { inwarn=0; c=0 }
            /^Warnings:/ { inwarn=1; next }
            inwarn && /^- / { c++ }
            END { print c }
        ' "$err_file")
    fi

    if [ "$error_count" -gt 0 ]; then
        status="SEMANTIC_ERROR"
    fi

    echo "${base},${ast_ok},${dot_ok},${png_ok},${ir_ok},${sym_ok},${error_count},${warning_count},${status}" >> "${SUMMARY_FILE}"
done

echo "Summary written to ${SUMMARY_FILE}"
cat "${SUMMARY_FILE}"
