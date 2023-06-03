import socket
from sys import exit
import threading

HOST = '0.0.0.0'  # listen on both loopback and local IP address
PORT = 8001

VERSOIN = "1.1" # Implemented blocking swear/curse words and removed invalid/inapropriate names

swearList = ['fuck', 'shit', 'bitch', 'ass', 'nigg']
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((HOST, PORT))
print(f"Server binded on {HOST} with port {PORT}")
server_socket.listen()

clients = []
names = []

def broadcast(message):
    #with open("../logs/chat.log", "a") as log_file:
    #    log_file.write(message.decode())
    for client in clients:
        client.send(message)
    with open("chat.log", "a") as log_file:
        log_file.write(message.decode())


def handle_client(client_socket, client_address):
    print(f'New connection from {client_address}')
    if client_address[0] in [c.getpeername()[0] for c in clients]:
        print(f'{client_address} is attempting to connect twice')
        client_socket.send("Your already connected\n".encode())
        client_socket.close()
        return
    clients.append(client_socket)
    client_socket.send("Name: ".encode())
    try:
        name = client_socket.recv(1024).decode().replace("\n", "")
        if name:
            if name == "" or " " in name or SwearOrNot(name):
                for c in clients:
                    c.send("Your name have problems\n".encode())
                    clients.remove(c)
                    c.close()
                    return
            for n in names:
                if n == name: # need test
                    for c in clients:
                        snames = " ".join(names)
                        c.send(f"Username taken, use an unused name, current names: {snames}\n".encode())
                        clients.remove(c)
                        c.close()
                    return
        else:
            raise Exception('Client disconnected without registering')
    except Exception as e:
        print(e)
        clients.remove(client_socket)
        print(f'{client_address} has left the chat without registering')
        #broadcast(f'{client_address} has left the chat\n'.encode())
        broadcast(f'{client_address} has left the chat without even registering\n'.encode())
        client_socket.close()
        return

    names.append(name)
    #broadcast(f'{client_address} has joined the chat\n'.encode())
    broadcast(f'{name} has joined the chat\n'.encode())
    #with open("../logs/chat.log", "r") as log_file:
    #    log_contents = log_file.read()
    #    client_socket.send(log_contents.encode())
    with open("chat.log", "r") as log_file:
        log_contents = log_file.read()
        client_socket.send(log_contents.encode())

    while True:
        try:
            message = client_socket.recv(1024)
            if message:
                rmsg = message.decode()
                if client_address[0] == "127.0.0.1" and rmsg.startswith("$") or client_address[0] == "192.168.1.16" and rmsg.startswith("$"):
                    command = rmsg[1:].strip()
                    #command_parts = message[1:].strip().split()
                    print(f"Command spawned: {command}")
                    if command == "list":
                        client_socket.send("Connected clients:\n".encode())
                        for c in clients:
                            c_address = c.getpeername()[0]
                            client_socket.send(f"{c_address}\n".encode())
                    elif "kick" in command and len(command.split()) == 2:
                        kick_address = command.split()[1]
                        if kick_address != "127.0.0.1":# and client_address[0] != "192.168.1.16":
                            for c in clients:
                                c_address = c.getpeername()[0]
                                if c_address == kick_address:
                                    c.send("You have been kicked\n".encode())
                                    clients.remove(c)
                                    c.close()
                                    client_socket.send(f"Kicked client at {kick_address} Succefully\n".encode())
                                    break
                                else:
                                    client_socket.send(f"No client found at {kick_address}".encode())
                    else:
                        client_socket.send("Unknown command or syntax \n".encode())

                if message.decode() != "":
                    #broadcast(f'{client_address}: {message.decode()}'.encode())
                    if SwearOrNot(message.decode()):
                        broadcast(f'{name}: {len(message.decode())*"#"}\n'.encode())
                    else:
                        broadcast(f'{name}: {message.decode()}'.encode())
                    print(f'{client_address}({name}): {message.decode()}')
            else:
                raise Exception('Client disconnected')
        except Exception as e:
            print(e)
            clients.remove(client_socket)
            print(f'{client_address} has left the chat')
            #broadcast(f'{client_address} has left the chat\n'.encode())
            broadcast(f'{name} has left the chat\n'.encode())
            client_socket.close()
            return

def SwearOrNot(var):
    if any(word in var for word in swearList):
        return True
    else:
        return False


def accept_connections():
    while True:
        try:
            client_socket, client_address = server_socket.accept()
            client_thread = threading.Thread(target=handle_client, args=(client_socket, client_address))
            client_thread.start()
        except KeyboardInterrupt:
            print("Closing server...")
            #client_thread.join()
            exit()

try:
    if __name__ == '__main__':
        with open("chat.log", "w") as log_file:
            log_file.truncate(0)
        try:
            server_thread = threading.Thread(target=accept_connections)
            server_thread.start()
            #client_socket, client_address = server_socket.accept()
            #handle_client(client_socket, client_address)
        except KeyboardInterrupt:
            print("Closing server...")
            #client_thread.join()
            exit()

        # Handle the server's own connection
        try:
            while True:
                try:
                    message = input('> ')
                    broadcast(f'{message}\n'.encode())
                except KeyboardInterrupt:
                    print("Closing server...")
                    #client_thread.join()
                    exit()
        except KeyboardInterrupt:
            print("Closing server...")
            #client_thread.join()
            exit()
except KeyboardInterrupt:
    print("Closing server...")
    #client_thread.join()
    exit()
