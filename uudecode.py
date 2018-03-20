#!/usr/bin/env python
from sys import argv
from uu import decode

decode(argv[1], argv[1].replace('.uue', ''))
