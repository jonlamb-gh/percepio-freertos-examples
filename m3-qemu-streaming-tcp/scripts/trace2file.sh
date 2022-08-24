#!/usr/bin/env bash

set -e

echo -ne '\x01\x01\x00\x00\x00\x00\xFD\xFF' | nc 192.0.2.80 8888 > /tmp/trace.psf

exit 0
