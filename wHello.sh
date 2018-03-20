#!/bin/sh
if [[ ! -d /sys/class/gpio/D13 ]]
then  echo 115 > /sys/class/gpio/export 2>/dev/null
fi
echo out > /sys/class/gpio/D13/direction
python /opt/lininoIO-REST/app.py & sleep 5
killw(){
	kill $!
	exit
}
trap killw SIGINT SIGTERM
w=http://127.0.0.1:8080/gpio/13
while :
do	wget -q $w/1 -O /dev/null
	sleep 1
	wget -q $w/0 -O /dev/null
	sleep 1
done
