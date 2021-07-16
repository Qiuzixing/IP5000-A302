#!/bin/bash
#echo "arm-aspeed-linux-gnu-strip sconfig"
#arm-aspeed-linux-gnu-strip $(pwd)/sconfig
#upx -qqq -v $(pwd)/sconfig
echo "arm-aspeed-linux-gnu-strip mainswitch"
arm-aspeed-linux-gnu-strip $(pwd)/../mainswitch/mainswitch
upx -qqq -v $(pwd)/../mainswitch/mainswitch

