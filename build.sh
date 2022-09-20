
gcc src/dict.c src/server.c src/sds.c src/zmalloc.c src/localtime.c src/config.c -o main.out
./main.out
rm -rf main.out
