tftp 169.254.0.2 -c get node_query
tftp 169.254.0.2 -c get node_responser
tftp 169.254.0.2 -c get responser_emu.sh

node_query Example:
---

```
# single key pair set
./node_query --if test --of test_of --set_key key1=value1
# multi key pair set
./node_query --if test --of test --set_key key1=value1
./node_query --if test --of test --set_key key2=value2
./node_query --if test --of test --set_key key3=value3
./node_query --if test --of test --set_key key4=value4

# dump
./node_query --if test --dump
# get key
./node_query --if test --get_key key1
# match key
./node_query --if test --match_key key1=value1 --get_key key2

# query from net
./node_query --of node_info --reply_type=full
./node_query --dump

# query from net for name resolve. Set --max to 1 to reply immediately.
./node_query --get_key MY_IP --match_key MY_MAC=02C7C324D7E3 --max 1
./node_query --get_key MY_IP --match_key MY_MAC=82900245BB17 --max 1

# A special command used to notify own changes. Where:
# - reply_type MUST BE NQ_NOTIFY_CHG
# - match_key MUST contain:
#   + FROM_MAC: notifyer's MAC addr for matching.
#   + SESSION: notifyer's session id. a random sequence number for avoiding duplication.
#   + CHG_INFO: the changed info file name. Ex: essential
./node_query --reply_type NQ_NOTIFY_CHG --match_key FROM_MAC=0030F1112233 --match_key SESSION=12345678 --match_key CHG_INFO=essential



# Special reply_type for emulation
./node_query --dump --reply_type emu
./node_query --get_key MY_MAC --reply_type emu

```

node_responser Example:
---

```
# multi key pair set for 'essential' reply_type
./node_query --if essential --of essential --set_key MY_MAC=82900245BB17
./node_query --if essential --of essential --set_key MY_IP=169.254.4.195
./node_query --if essential --of essential --set_key IS_HOST=n
./node_query --if essential --of essential --set_key HOSTNAME="Im1500"
./node_query --if essential --of essential --set_key MULTICAST_ON=y
./node_query --if essential --of essential --set_key JUMBO_MTU=8000
./node_query --if essential --of essential --set_key CH_SELECT=0002

# start node_responser
./node_responser --mac 82900245BB17
```

node_responser match test cases:
---
- query buf: 空, response buf: 不可能是空的，只能是 MY_MAC only
- query buf: MY_MAC, resp buf: MY_MAC only  ==> match cases
- query buf: MY_MAC, resp buf: MY_MAC only  ==> not match cases
- query buf: MA_MAC, resp buf: MY_MAC + MY_IP  ==> match cases
- query buf: MA_MAC, resp buf: MY_MAC + MY_IP  ==> not match cases
- query buf: MA_MAC, resp buf: MY_IP + MY_MAC  ==> match cases
- query buf: MA_MAC, resp buf: MY_IP + MY_MAC  ==> not match cases
- query buf: MA_MAC, resp buf: MY_IP + TEST1 + MY_MAC  ==> match cases
- query buf: MA_MAC, resp buf: MY_IP + TEST1 + MY_MAC  ==> not match cases
- query buf: MA_MAC, resp buf: MY_IP + TEST1 + MY_MAC + TEST2 ==> match cases
- query buf: MA_MAC, resp buf: MY_IP + TEST1 + MY_MAC + TEST2  ==> not match cases
- query buf: MA_MAC+MY_IP, resp buf: MY_IP + TEST1 + MY_MAC  ==> match cases
- query buf: MA_MAC+MY_IP, resp buf: MY_IP + TEST1 + MY_MAC  ==> MY_IP not match cases
- query buf: MA_MAC+MY_IP, resp buf: MY_IP + TEST1 + MY_MAC  ==> 2 not match cases
