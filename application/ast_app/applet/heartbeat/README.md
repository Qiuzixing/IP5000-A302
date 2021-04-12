start program:
---
```
./heartbeat &
```

exist:
---
```
./ipc @hb_ctrl s exit
```


start pinger:
---
```
# Multicast:
# start_pinger:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
./ipc @hb_ctrl s start_pinger:v:1234:mc:225.0.100.011:99

# Unicast
# start_pinger:type:session_id:uc:target_acker_ip:target_and_listen_port
./ipc @hb_ctrl s start_pinger:v:1234:uc:169.254.135.254:99

./ipc @hb_ctrl s start_pinger:u:1234:uc:169.254.10.133:99
./ipc @hb_ctrl s start_pinger:v:1234:uc:169.254.10.133:100
```

stop pinger:
---
```
./ipc @hb_ctrl s stop_pinger:u:1234
```

start acker:
---
```
# Multicast
# start_acker:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
./ipc @hb_ctrl s start_acker:v:1234:mc:225.0.100.011:99

# Unicast
# start_acker:type:session_id:uc:acker_ip:acker_target_and_listen_port
./ipc @hb_ctrl s start_acker:u:1234:uc:169.254.135.254:99

./ipc @hb_ctrl s start_acker:u:1234:uc:169.254.10.133:99
./ipc @hb_ctrl s start_acker:v:1234:uc:169.254.10.133:100
```

stop acker:
---
```
./ipc @hb_ctrl s stop_acker:v:1234
```


msg_channel:
---
type: to assign which channel to use. received message will be fired to this type. See "Type List" below.
to_whom: to assign whom to receive:
	- acker:
	- pinger:
specific_ip:
	- xxx.xxx.xxx.xxx: specify which IP to receive.
		+ Use 255.255.255.255 means not specified.
		+ Use 0.0.0.0 for acker. Means specified 'HB session ID'.
```
# msg:type:to_whom:specific_ip:msg_content
ipc @hb_ctrl s msg:v:acker:255.255.255.255:ve_btn1_pressed
ipc @hb_ctrl s msg:u:acker:255.255.255.255:ue_request_usb:$MY_IP
ipc @hb_ctrl s msg:u:pinger:169.254.0.33:ue_stop_usb
ipc @hb_ctrl s msg:u:pinger:169.254.0.34:ue_start_usb

```

Type List:
---
- v: video
- a: audio
- u: usb
- r: IR over ip
- s: serial over ip
- p: push button (gpio) over ip
- c: cec over ip. TBD
- g: god mode. broadcaster. For broadcasting messages. TBD.

Notice:
---
- unicast下，acker不同feature的port需要分開，不然無法支援 multi pinger to one acker的架構。