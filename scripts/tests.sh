#!/usr/bin/env bash

function check_test {
  echo "++++++++++++++++++++++++++"
  echo "Running $1"
  "$@"
  local status=$?
  if [ $status -ne 0 ]; then
    echo "Test $1 has failed."
    popd > /dev/null
    exit -1
  fi
  echo "++++++++++++++++++++++++++"
  return $status
}


pushd ../bin > /dev/null

# Run all tests
check_test ./test_powers
check_test ./test_underflow
check_test ./test_overflow

popd > /dev/null

echo "All tests have passed."
