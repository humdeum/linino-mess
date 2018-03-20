#!/bin/sh

scg=/sys/class/gpio

echo 114 > $scg/export
echo 123 > $scg/export

echo out > $scg/D5/direction
echo out > $scg/D6/direction

f_en=$scg/D6/value
freq=$scg/D5/value

for b in 1 1 0 0 ;do
	echo 1 > $f_en
	sleep 1
	echo $b > $freq
	sleep 1
	echo 0 > $f_en
	sleep 1
done

echo 114 > $scg/unexport
echo 123 > $scg/unexport
