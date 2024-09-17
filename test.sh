#!/bin/bash

# Simple test of the command interpreter

in="abbabaq"
out="0,3,5;"

[[ $(./cmd_int <<< "$in") == "$out"* ]] && echo "Test 1 PASSED" || echo "Test 2 FAILED"


in="aacq"
out="0;"
[[ $(./cmd_int <<< "$in") == "$out"* ]] && echo "Test 2 PASSED" || echo "Test 2 FAILED"


in="ababaaq"
out="0,2,4,5;"
[[ $(./cmd_int <<< "$in") == "$out"* ]] && echo "Test 3 PASSED" || echo "Test 3 FAILED"


# Use this code in the terminal to run the tests: ./test.sh