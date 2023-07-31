import subprocess  
import threading 
import time
  
# 定义要连接的服务端IP和端口  
server_ip = '127.0.0.1'  
server_port = 3000  
  
# 定义要发送的数字数据  
data = [1, 2, 3]  
  
# 定义一个函数来执行nc命令并发送数据  
def send_data():  
    # 构造nc命令  
    cmd = f"nc {server_ip} {server_port}"  
  
    # 创建子进程并执行nc命令  
    p = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE)  
  
    # 通过标准输入流发送数据到服务端  
    for d in data:  
        p.stdin.write(f"{d}\n".encode())  
        p.stdin.flush() 
        # 每次循环之间延时d秒 模拟等待服务端响应。
        time.sleep(d)  
  
    # 关闭标准输入流  
    p.stdin.close()  
  
    # 等待子进程结束  
    #p.wait()  
  
    # 手动退出子进程  
    #p.terminate()  
  
# 创建一个线程列表  
threads = []  
  
# 循环创建并启动线程  
for i in range(10):  
    t = threading.Thread(target=send_data)  
    t.start()  
    threads.append(t)  
  
# 等待所有线程执行完毕  
for t in threads:  
    t.join()  