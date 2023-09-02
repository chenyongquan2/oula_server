import socket  
import time  
  
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
  
server_address = ('localhost', 1234)  
client_socket.connect(server_address)  
#client_socket.setblocking(0)  

total_size = 10000000
data1 = 'a' * total_size 

n1 = client_socket.send(data1.encode())  # Encode the string to bytes before sending  

#sleep to mock the client not recv the data from server on purpose, 
#result to the writeBuf filled with full data ,and not space to write continue. 
time.sleep(5)
times=0
once_bytes=total_size//10
while True:  
    chunk = client_socket.recv(once_bytes)  
    if not chunk:
        break
    times+=1
    print('recv times:{}'.format(times))  
    #print('recv: {}'.format(chunk.decode()))  # Decode the received bytes to string  
    time.sleep(0.1)  
  
print('recv all')  