#!/bin/sh

export PATH=$PATH:$BASE_DIR/../output/host/bin

cp $BASE_DIR/../custom-scripts/S41network-config $BASE_DIR/target/etc/init.d
chmod +x $BASE_DIR/target/etc/init.d/S41network-config

i686-linux-gcc $BASE_DIR/../custom-scripts/hello.c -O2 -o $BASE_DIR/../output/target/usr/bin/hello
make -C $BASE_DIR/../modules/simple_driver/