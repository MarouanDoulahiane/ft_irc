import socket
import random
import string
import time
import threading

def randomString(stringLength=10):
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(stringLength)) 

def test_registration():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 6667))
    nick = randomString(8)
    user = randomString(8)
    password = "1234"
    s.send(f"PASS {password}\r\n".encode())
    s.send(f"NICK {nick}\r\n".encode())
    s.send(f"USER {user} 0 * :{user}\r\n".encode())
    data = s.recv(1024)
    print(data.decode())

def run_test():
    while True:
        test_registration()
        time.sleep(1)

# Create multiple threads
num_threads = 10
threads = []
for _ in range(num_threads):
    t = threading.Thread(target=run_test)
    threads.append(t)
    t.start()

# Wait for all threads to finish
for t in threads:
    t.join()
