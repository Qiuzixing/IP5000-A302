@echo off

IF "%1"=="" GOTO no_local_intf

@node_query.exe --json --get_key HOSTNAME --get_key MY_IP --intf "%1"
GOTO end

:no_local_intf
@node_query.exe --json --get_key HOSTNAME --get_key MY_IP

:end
@pause