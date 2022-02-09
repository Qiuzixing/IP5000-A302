ipc_server
---
a ipc server to bridge two ipc client.

NOTE:
- Query/Reply uses flock() to make sure queries are in sequence per channel.
  The one be queried must set reply in the same context being queried.
  And reply ASAP. Be ware, recursive query will dead lock the system.

Example:
```
# Start ipc_server. Start 4 ipc server.
# @v_lm_set for input from user. (ipc s)
# @v_lm_get for output to ipc client. (ipc g)
# @v_lm_query for query from user. and synchronously waiting for reply. (ipc q)
# @v_lm_reply for reply to user (ipc s)
./ipc_server @v_lm_set @v_lm_get @v_lm_query @v_lm_reply &
# Use ipc command to talk to each other
./ipc @v_lm_set s hello
./ipc @v_lm_get g

# Query/Reply Example
./ipc_server @v_lm_set @v_lm_get @v_lm_query @v_lm_reply &
./ipc @v_lm_get g
./ipc @v_lm_query q "query string"
./ipc @v_lm_reply s "reply string"
```

Test Pattern:
---
```
# 1
./ipc @v_lm_query q "query string"
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_reply s "reply string"
# 2
                                       ./ipc @v_lm_get g
./ipc @v_lm_query q "query string"
                                       ./ipc @v_lm_reply s "reply string"
# 3
                                       ./ipc @v_lm_get g
./ipc @v_lm_query q "query string" &
./ipc @v_lm_query q "query string 2"
                                       ./ipc @v_lm_reply s "reply string 1"
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_reply s "reply string 2"
# 3.1
                                       ./ipc @v_lm_get g
./ipc @v_lm_query q "query string" &
./ipc @v_lm_query q "query string 2"
                                       ./ipc @v_lm_reply s "reply string 1"
                                       ./ipc @v_lm_set s "set string"
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_reply s "reply string 2"
                                       ./ipc @v_lm_get g
# 3.2
                                       ./ipc @v_lm_get g
./ipc @v_lm_query q "query string" &
./ipc @v_lm_query q "query string 2"
                                       ./ipc @v_lm_reply s "reply string 1"
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_set s "set string"
                                       ./ipc @v_lm_reply s "reply string 2"
                                       ./ipc @v_lm_get g
# 3.3
                                       ./ipc @v_lm_get g
./ipc @v_lm_query q "query string" &
./ipc @v_lm_query q "query string 2"
                                       ./ipc @v_lm_reply s "reply string 1"
                                       ./ipc @v_lm_set s "set string"
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_reply s "reply string 2"

# 4
                                       ./ipc @v_lm_get g
./ipc @v_lm_query q "query string 1" &
./ipc @v_lm_query q "query string 2"
                                       ./ipc @v_lm_reply s "reply string 1"
                                       ./ipc @v_lm_reply s "reply string 2"
                                       ./ipc @v_lm_get g
# 5
                                       ./ipc @v_lm_get g
./ipc @v_lm_query q "query string"
                                       ./ipc @v_lm_set s "set string"
                                       ./ipc @v_lm_reply s "reply string"
                                       ./ipc @v_lm_get g
# 6
                                       ./ipc @v_lm_get g
./ipc @v_lm_query q "query string"
                                       ./ipc @v_lm_set s "set string"
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_reply s "reply string"
# 7
                                       ./ipc @v_lm_get g
./ipc @v_lm_query q "query string"
                                       ./ipc @v_lm_set s "set string 1"
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_set s "set string 2"
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_reply s "reply string"
# 8
                                       ./ipc @v_lm_get g
./ipc @v_lm_set s "set string 1"
./ipc @v_lm_set s "set string 2"
                                       ./ipc @v_lm_get g

./ipc @v_lm_query q "query string"
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_reply s "reply string"
# 9
                                       ./ipc @v_lm_get g
./ipc @v_lm_set s "set string 1"
                                       ./ipc @v_lm_get g
./ipc @v_lm_set s "set string 2"

./ipc @v_lm_query q "query string"
                                       ./ipc @v_lm_get g
                                       ./ipc @v_lm_reply s "reply string"
# 10
                                       ./ipc @v_lm_get g
./ipc @v_lm_set s "set string 1"
                                       ./ipc @v_lm_get g
./ipc @v_lm_set s "set string 2"
                                       ./ipc @v_lm_get g
./ipc @v_lm_query q "query string"
                                       ./ipc @v_lm_reply s "reply string"
# 11
                                       ./ipc @v_lm_get g
./ipc @v_lm_get g &
./ipc @v_lm_set s "set string 1"
./ipc @v_lm_set s "set string 2"
./ipc @v_lm_set s "set string 3"
./ipc @v_lm_set s "set string 4"
./ipc @v_lm_set s "set string 5"
./ipc @v_lm_set s "set string 6"
./ipc @v_lm_set s "set string 7"
./ipc @v_lm_set s "set string 8"
./ipc @v_lm_set s "set string 9"
./ipc @v_lm_set s "set string 10"
./ipc @v_lm_set s "set string 11"
./ipc @v_lm_set s "set string 12"
./ipc @v_lm_set s "set string 13"

```