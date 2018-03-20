#!/usr/bin/env python
from sys import argv
import tarfile

f = tarfile.open(argv[1])
f.extractall()
f.close()
