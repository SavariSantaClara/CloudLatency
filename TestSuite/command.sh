rm test;
gcc Common.c Socket.c Mqtt.c TestMain.c -I. -g -o test -lmosquitto
