#!/bin/bash

echo "=== 1. First Clean all tmp Files ==="
make clean
echo ""
echo "=== 2. Build all testcase LLVM IR Files ==="
make Test
echo ""
echo "=== 3. Build all pass Files/Libraries ==="

if [ $# -eq 0 ]; then
    echo "Please specify the parameter: hello, demo or dependence"
    echo "(EX. './run.sh dependence' for HW1)"
    echo ""

elif [ $1 == "dependence" ]; then 
    echo "--- HW1 Pass Run ---"
    echo "--- build hw1.so ---"
    make hw1.so
    echo ""
    echo "--- Run testcase ---"
    echo ""
    echo "[testcase1]"
    echo ""
    opt -load ./hw1.so -dependence test1.ll -o /dev/null
    echo ""
    echo ""
    echo "[testcase2]"
    echo ""
    opt -load ./hw1.so -dependence test2.ll -o /dev/null
    echo ""
    echo ""
    echo "[testcase3]"
    echo ""
    opt -load ./hw1.so -dependence test3.ll -o /dev/null
    echo ""
    echo ""
    echo "--- End testcase ---"

elif [ $1 == "hello" ]; then 
    echo "--- hello Pass Run ---"
    echo "--- build hello_pass.so ---"
    make hello_pass.so
    echo "--- Run testcase ---"
    echo "[testcase1]"
    opt -load ./hello_pass.so -hello test1.ll -o /dev/null
    echo "[testcase2]"
    opt -load ./hello_pass.so -hello test2.ll -o /dev/null
    echo "[testcase3]"
    opt -load ./hello_pass.so -hello test3.ll -o /dev/null
    echo "--- End testcase ---"

elif [ $1 == "demo" ]; then 
    echo "--- Demo Pass Run ---"
    echo "--- build demo.so ---"
    make Demo.so
    echo "--- Run testcase ---"
    echo "[testcase1]"
    opt -load ./Demo.so -Demo test1.ll -o /dev/null
    echo "[testcase2]"
    opt -load ./Demo.so -Demo test2.ll -o /dev/null
    echo "[testcase3]"
    opt -load ./Demo.so -Demo test3.ll -o /dev/null
    echo "--- End testcase ---"

fi
