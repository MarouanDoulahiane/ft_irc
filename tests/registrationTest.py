# create a test i mean try to register to irc server from randomusers

# localhost:6667
# password: 1234

import socket
import random
import string
import time

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

while True:
    test_registration()
    time.sleep(0.1)
