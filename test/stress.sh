#!/bin/bash
#
# Concurrent connection stress test.
PORT=8080
siege -c 2 -r 10 http://127.0.0.1:$PORT/
