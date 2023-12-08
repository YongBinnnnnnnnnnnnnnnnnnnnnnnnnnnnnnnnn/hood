#!/bin/bash
curl -s "https://open-vsx.org/api/$1" | sed -r -e "s/.*download\"\:\"([^\"]*).*/\1/"|xargs curl -L -O -C -
