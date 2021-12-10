#!/usr/bin/env bash

# Test that overflow and underflow is detecatble on the system
./tests.sh

if [ $? -ne 0 ]; then
  echo "Tests have failed."
  exit -1
fi

echo ""

pushd ../bin > /dev/null

# Run identify
./rootcat ../data/identify | ./identify

if [ $? -eq 0 ]; then
  echo -e "Identification succeded.\n"
  # List unique intersecions for ease of checking
  echo "Rerunning to list unique variety intersections."
  ./rootcat ../data/identify | ./identify | grep "Valid" | sort -u
else
  echo "Identification failed."
  popd > /dev/null
  exit -1
fi

popd > /dev/null
