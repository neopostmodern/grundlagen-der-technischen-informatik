#! /bin/sh

echo 'Starting a bunch of servers. Expect output...'

../42/HashServer localhost 8000   0 localhost 8001 &
SERVER_1=$!
../42/HashServer localhost 8001  64 localhost 8002 &
SERVER_2=$!
../42/HashServer localhost 8002 128 localhost 8003 &
SERVER_3=$!
../42/HashServer localhost 8003 196 localhost 8000 &
SERVER_4=$!

echo 'Hit [ENTER] to kill.'
read _

echo 'Shutting down...'
kill $SERVER_1
kill $SERVER_2
kill $SERVER_3
kill $SERVER_4

exit 0
