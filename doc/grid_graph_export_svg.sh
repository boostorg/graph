#!/bin/bash

# Unindexed, unwrapped
inkscape --export-png grid_graph_unwrapped.png --export-id g3150 --export-id-only grid_graph_unindexed.svg

# Unindexed, wrapped
inkscape --export-png grid_graph_wrapped.png grid_graph_unindexed.svg

# Indexed, unwrapped
inkscape --export-png grid_graph_indexed.png grid_graph_indexed.svg