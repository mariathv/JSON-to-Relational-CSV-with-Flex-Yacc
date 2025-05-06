#!/bin/bash

# Create output directory
mkdir -p output

# Run each test case
for i in {1..5}; do
    echo "Running test$i.json..."
    ./json2relcsv < test$i.json --out-dir output/test$i
    if [ $? -eq 0 ]; then
        echo "Test $i completed successfully"
        echo "Output files:"
        ls -l output/test$i/
    else
        echo "Test $i failed"
    fi
    echo
done 