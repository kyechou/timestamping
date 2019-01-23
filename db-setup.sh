#!/bin/bash

set -e

SCRIPT_DIR="$(dirname $(realpath ${BASH_SOURCE[0]}))"
cd "$SCRIPT_DIR"

mysql -h mustang01 -u demo -p password -D demo <src/db-setup.sql
