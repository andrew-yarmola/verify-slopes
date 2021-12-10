#!/usr/bin/env bash

# Test that overflow and underflow is detecatble on the system
./tests.sh

if [ $? -ne 0 ]; then
  echo "Tests have failed."
  exit -1
fi

echo ""

pushd ../bin > /dev/null

# Run verify
./rootcat ../data/verify | ./verify

if [ $? -eq 0 ]; then
  echo "Identification succeded."
else
  echo "Identification failed."
  popd > /dev/null
  exit -1
fi

popd > /dev/null
