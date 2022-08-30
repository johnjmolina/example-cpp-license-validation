#!/bin/bash
clang++ main.cpp -o bin.out \
        -std=c++17 \
        -stdlib=libc++ \
        -framework CoreFoundation \
        -framework IOKit \
        -lcpprest \
        -I/opt/homebrew/include \
        -L/opt/homebrew/lib \
        -lboost_system \
        -lboost_thread-mt \
        -lboost_chrono-mt \
        -lssl \
        -lcrypto \
        -I/opt/homebrew/opt/openssl/include \
        -L/opt/homebrew/opt/openssl/lib \
        -I/opt/homebrew/opt/cpprestsdk/include \
        -L/opt/homebrew/opt/cpprestsdk/lib

./bin.out kapsel_license

