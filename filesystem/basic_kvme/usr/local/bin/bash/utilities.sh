
enable -f /usr/local/bin/bash/sleep sleep
# Bruce160826. bash cat uses only 1024KB buf. It may causes sysfs access problem which requires PAGE_SIZE(4KB) in one write.
# TODO. Port Busybox's cat into bash!?
#enable -f /usr/local/bin/bash/cat cat

