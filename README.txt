==========================
GPL Guide
==========================

Model Name: WAP4410N
Customer: Cisco
Building OS: Ubuntu 8.10, Ubuntu 10.04

Source Code Structure
===========================

./
|-- Readme.txt                      This file
`-- src                             Source code
    |-- Result                      Image file is here 
    |-- apps                        Applications
    |-- build                       Build directory
    |-- build_wap4410n.script       Build script
    |-- linux                       Linux kernel
    |-- toolchain                   Compile toolchain
    `-- u-boot                      Boot loader

Build Steps
=====================

1. Change directory to src/

	# cd ./src

2. Run build_wap4410n.script in shell to build the toolchain, boot loader and fw image file.

	# ./build_wap4410n.script

3. Get image file in ./src/Result/, and enjoy it!

