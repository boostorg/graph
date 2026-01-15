#!/bin/bash

set -e

BENCHMARK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$BENCHMARK_DIR"

# Activate virtual environment if it exists
if [ -d "venv" ]; then
    source venv/bin/activate
fi

mkdir -p outputs

echo "Building BGL Louvain..."
g++ -std=c++17 -O3 -I/opt/homebrew/include -I../../include -o bgl_louvain bgl_louvain.cpp

echo "Running benchmarks..."
python3 benchmark.py

echo "Generating plots..."
python3 visualize.py

echo "Done. Results in outputs/"
