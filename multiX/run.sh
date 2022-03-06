#!/bin/bash
python3 /home/remi/.local/projects/hacknetGtk/multiX/display/daemon.py &
cd augmentation
sleep 1
make run
