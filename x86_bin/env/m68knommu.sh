PATH="$PATH::$PWD/host_bin:$PWD/toolchain/freescale-coldfire-2011.09/bin"
#PATH="$PATH::$PWD/host_bin:$PWD/toolchain/freescale-coldfire-4.4/bin/"
CROSS_COMPILE="$PWD/toolchain/freescale-coldfire-2011.09/bin/m68k-uclinux-"
#CROSS_COMPILE="$PWD/toolchain/freescale-coldfire-4.4/bin/m68k-uclinux-"
export CROSS_COMPILE
ARCH=m68knommu
export ARCH
