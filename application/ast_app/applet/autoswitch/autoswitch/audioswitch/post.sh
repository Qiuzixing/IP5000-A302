#!/bin/bash
#echo "arm-aspeed-linux-gnu-strip sconfig"
#arm-aspeed-linux-gnu-strip $(pwd)/sconfig
#upx -qqq -v $(pwd)/sconfig
echo "arm-aspeed-linux-gnu-strip audioswitch"
arm-aspeed-linux-gnu-strip $(pwd)/../audioswitch/audioswitch
upx -qqq -v $(pwd)/../audioswitch/audioswitch

