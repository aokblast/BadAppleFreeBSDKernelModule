#!/usr/bin/env bash

mkdir /etc/badapple/
cp ./frames/* /etc/badapple/
make
make install
