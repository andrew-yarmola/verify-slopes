#!/usr/bin/env bash

pushd ../src
# build
make test_float
make rootcat
make verify
make identify
popd
