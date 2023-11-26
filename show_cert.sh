#!/bin/sh
openssl s_client --connect $1:443 -showcerts </dev/null