#!/bin/bash
#
# Concurrent connection stress test.
PORT=8080
siege -c 200 -r 1 http://127.0.0.1:$PORT/
