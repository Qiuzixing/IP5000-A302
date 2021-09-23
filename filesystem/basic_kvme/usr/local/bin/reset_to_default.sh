#/bin/sh
cd /usr/local/bin

echo "Reset to factory default..."
./astparam flush
./astparam save

rm /data/configs/kds-7/ -rf
echo "done"
