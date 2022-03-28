import socket
from threading import Thread
import time
import sys
import argparse

HOST = "127.0.0.1"
request = "GET /index.html"
parser = argparse.ArgumentParser()
parser.add_argument("-r", "--requests", type=int, default=1000)
parser.add_argument("-p", "--port", type=int, default=5080)
parser.add_argument("-v", "--verbose", action="store_true")
parser.add_argument("-f", "--file", action="store_true")

args = parser.parse_args()
PORT = args.port
verbose = args.verbose
print_response = args.file
number_of_requests = args.requests
threads = []


def req(id):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(request.encode())
        response = s.recv(1024)

        if verbose:
            print(id)

        if print_response:
            print(response.decode())


start = time.time()
for i in range(number_of_requests):
    thread = Thread(target=req, args=(i,))
    threads.append(thread)
    thread.start()

for thread in threads:
    thread.join()

end = time.time()

print("Finished!")
print(f"\n{number_of_requests} requests took {(end - start)*1000:.0f} ms")