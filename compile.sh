#!/bin/bash/
cd gemtreewriter
make all -j4
cd ../dqm-root/
make all -j4
cd ../
