#!/bin/bash
for filename in tests/*/*.mi; do
    ./vm_riskxvii $filename <${filename%.*}.in | diff ${filename%.*}.out -
done