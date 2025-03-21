#!/bin/sh

if [ $# -ne 2 ]; then
    echo "Usage: $0 <ip> <port>"
    exit 1
fi

while true; do
  exec 3<>/dev/tcp/$1/$2
  echo "test" >&3
done
