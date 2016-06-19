#!/bin/bash
#
# Concurrent connection stress test.
PORT=8080
siege -c 100 -r 10 http://localhost:$PORT/
