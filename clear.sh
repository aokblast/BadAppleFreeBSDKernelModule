#!/usr/bin/env bash

kldunload badapple_lkm.ko
rm -rf /etc/badapple
make clean
