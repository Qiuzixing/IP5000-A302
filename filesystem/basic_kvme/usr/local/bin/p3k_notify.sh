#! /bin/sh
kill_process()
{
	#2021.10.12 qzx:If the process starts with parameters, the pkill -9 process_name cannot be killed.You must use kill -9 to kill the process started with parameters
	PROCESS_PID=`ps | grep $1 | grep -v grep | awk '{print $1}'`
	if [ $PROCESS_PID ];then
		kill -9 $PROCESS_PID
	fi
}

kill_process p3k_notify_timer.sh
p3k_notify_timer.sh $1 &