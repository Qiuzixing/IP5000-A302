#!/bin/sh

#
# Usage:
#  pll [-i <Fin>] [-s <en>] [-p <ppm>] [-t <target clock>] [-f <file name>]
#
#    -i <Fin>            System clock input in Hz (24000000 or 25000000). Default:24000000
#    -s <en>             Enable fractional mode (1 or 0). Default:1
#    -p <ppm>            Target tolerance in PPM. Default:5000
#    -t <target clock>   Desired output clock in 100Hz (ex: 27MHz == 270000). Default: built-in table
#    -f <file name>      Desired output clock in 100Hz list in file format. See pll.txt for example.
#    -h                  Help
#

# To generate Fin:24MHz fractional pixel clock for CRT
./a.out -i 24000000 -s 1 -p 5000 -f crt.txt

# To generate Fin:24MHz fractional M-clock for I2S
./a.out -i 24000000 -s 1 -p 1000 -f i2s.txt

# To generate Fin:25MHz fractional pixel clock for CRT 
./a.out -i 25000000 -s 1 -p 5000 -f crt.txt

# To generate Fin:25MHz fractional M-clock for I2S
./a.out -i 25000000 -s 1 -p 1000 -f i2s.txt

# See details of a 81920*100 Hz pll setting
./a.out -i 24000000 -s 1 -p 5000 -t 81920
