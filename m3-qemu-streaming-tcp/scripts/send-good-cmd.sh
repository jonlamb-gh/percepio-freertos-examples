#!/usr/bin/env bash

set -e

echo -ne '\xAA\xBB\xC8\xCC' > /dev/udp/192.0.2.80/9999

exit 0
