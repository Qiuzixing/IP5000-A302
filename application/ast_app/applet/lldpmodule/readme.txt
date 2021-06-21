conf.mak 配置编译信息
out ：为编译库，头文件，及daemon进程输出目录 


如果编译报libevent中  aclocal-1.16 command not fand 错误

在 libevent 目录下 执行 ：sudo autoreconf -ivf

执行daemon

1.daemon后台进程：lldpd

判断 安装路径/var/run/是否存在，不存在则创建(用来存储daemon pid,及和daemon通信的本地套接字)
 如：编译时选择的安装路径为/home/longzhang/install
则是/home/longzhang/install/var/run
没有则：mkdir -p /home/longzhang/install/var/run
运行：./lldpd -I eth0

 -I: 监听发送网口
 -d： 调试信息
 


 2.调用控制库接口进行控制