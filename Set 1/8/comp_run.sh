#!/bin/bash

# Needed for string_view
g++ -std=c++17  ./sol_blockSizeIsAlways16YouMoron.cc -lssl -lcrypto  && ./a.out
