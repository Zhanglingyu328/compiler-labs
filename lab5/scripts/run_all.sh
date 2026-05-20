#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.."
make
./lab5_semantic --batch examples -o output
