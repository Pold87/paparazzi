#!/bin/sh
host=192.168.1.1
port=23
login=notneeded
passwd=notneeded
cmd="killall -9 test"

echo open ${host} ${port}
sleep 0.1
echo ${cmd}
sleep 0.1
echo exit
