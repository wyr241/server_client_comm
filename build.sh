#!/bin/sh

set -xe

g++ client.cpp -o client -std=c++11 -pthread
g++ server.cpp -o server -std=c++11 -pthread