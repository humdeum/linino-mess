#!/usr/bin/env python
import os
import time

if not os.path.exists('/sys/class/gpio/D13'):
	export = open('/sys/class/gpio/export', 'w')
	print('Enabling D13')
	write(export, '115')
	export.close()

with open('/sys/class/gpio/D13/direction', 'w') as f:
	f.write('out')

while True:
	with open('/sys/class/gpio/D13/value', 'w') as f:
		f.write('1\n')
		print('1')
		f.close()
		time.sleep(1)

	with open('/sys/class/gpio/D13/value', 'w') as f:
		f.write('0\n')
		print('0')
		f.close()
		time.sleep(1)
	

