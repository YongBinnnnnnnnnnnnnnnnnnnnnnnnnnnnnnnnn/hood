#!/bin/bash

rm -r data/filters
./AdGuardHome >> a.log 2>&1 &
