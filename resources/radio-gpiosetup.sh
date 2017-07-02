#! /bin/sh

gpio edge 27 rising
gpio edge 17 rising
gpio edge  4 rising
gpio edge 22 rising

gpio export 23 out
gpio export 18 out
gpio export 24 out
gpio export 25 out
gpio export 8 out
gpio export 7 out

gpio -g mode 11 out
gpio -g write 11 on

sleep 1
