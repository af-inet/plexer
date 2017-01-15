#!/bin/bash
#
# Concurrent connection stress test.
PORT=8080
siege -c 100 -r 1 http://127.0.0.1:$PORT/
