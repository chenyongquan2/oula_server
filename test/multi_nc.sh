#!/bin/bash  
  
# 服务器地址和端口  
server="0.0.0.0"  
port=3000  
clientCnt=10

# 循环执行nc命令  
for ((i=1; i<=$clientCnt; i++))  
do  
    
    # 发送数据到服务器并发送消息  
    nc "-v" "$server" "$port" & echo "1" & echo "2" & echo "3"
  
    # 可选：加入延时  
    sleep 1  
done  