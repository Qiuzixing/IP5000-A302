在x86 Linux下跑Client連線的模擬動作
---

修改:
---
- [Host] msg_channel::handle_new_connection()裡的get_client_fd_by_ip()要跳掉。不然Host會把同一台連上的Client給踼掉。
- 各個driver對應app的修改。看#define SIM 1。

Compile and Build:
- 到各目錄下，打make。會compile x86的binary出來。並放到此目錄。
- 把整個此目錄放到x86 Linux環境下執行。

執行:
---
- 看start.sh和test.sh。


