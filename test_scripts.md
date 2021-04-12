client 初始
```
e e_var;sleep 1

link_mgrc_video.sh
ipc @v_lm_set s ve_test
```

client 重跑
```
ipc @hb_ctrl s stop_pinger:v

ipc @v_lm_set s ve_lm_destroy
ipc @hb_ctrl s exit

tftp 169.254.0.2 -c get link_mgrc_video.sh

link_mgrc_video.sh
ipc @v_lm_set s ve_test

```

client 測試:
```
# start
ipc @v_lm_set s ve_start:0011:225.0.100.011
# stop
ipc @v_lm_set s ve_stop
```


host 初始
```
e e_var;sleep 1

link_mgrh_video.sh
ipc @v_lm_set s ve_test
```

host 測試:
```
# start
ipc @v_lm_set s ve_start:0011:225.0.100.011
# stop
ipc @v_lm_set s ve_stop





