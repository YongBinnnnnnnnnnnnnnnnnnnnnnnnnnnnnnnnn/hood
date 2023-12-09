#!/bin/sh

/usr/local/bin/hood-http-handler.py >> /var/log/hood-http-handler.log &
/usr/local/bin/hood-name-service.py >> /var/log/hood-name-service.log &
#/usr/local/bin/hood-tls-proxy >> /var/log/hood-tls-proxy.log 