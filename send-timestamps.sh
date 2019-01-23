#!/bin/bash

set -e

SCRIPT_DIR="$(dirname $(realpath ${BASH_SOURCE[0]}))"
cd "$SCRIPT_DIR"

cat tsdump.*.sql | mysql -h mustang01 -u demo -ppassword -D demo
