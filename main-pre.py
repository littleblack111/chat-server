
#  Copyright (C) 2023 littleblack111 <https://github.com/littleblack111/>

import socket
from os import path
from sys import exit
import threading

HOST = '0.0.0.0'  # listen on both loopback and local IP address
PORT = 8000

VERSION = "1.2" # Implemented mute, unmute and tempmute, added userlist
HOME = path.expanduser('~')

swearList = ['fuck', 'shit', 'bitch', 'nigg']
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((HOST, PORT))
print(f"Server binded on {HOST} with port {PORT}")
server_socket.listen()

clients = []
names = []
userlist = []
cmdlist = ['list', 'kick', 'mute', 'unmute', 'tmpmute', 'tempmute']
sweart1 = []
sweart2 = []
muted = []

def stop(self):
    client_thread.join()
    server_thread.join()
    self.running = False
    socket.socket(socket.AF_INET, 
                  socket.SOCK_STREAM).connect( (self.hostname, self.port))
    self.socket.close()

def broadcast(message):
    for client in clients:
        client.send(message)
    with open(f"{HOME}/scripts/store/chat-server/chat.log", "a") as log:
        try:
            log.write(message.decode())
        except AttributeError:
            log.write(message)
    #with open("chat.log", "a") as log:
        #log.write(message.decode())

def kick(address, kuname, client_socket):
    global faddress
    for c in clients:
        c_address = c.getpeername()[0]
        if c_address == address:
            c.send("You have been kicked\n".encode())
            clients.remove(c)
            c.close()
            names.remove(kuname)
            userlist.remove(kuname)
            if faddress:
                address = userlist[userlist.index(address)-1]
                userlist.remove(address)
                faddress = False
            try:
                muted.remove(kuname)
            except ValueError:
                pass
            client_socket.send(f"Kicked client at {address} Succefully\n".encode())
            try:
                broadcast(f"{address} is kicked")
            except:
                pass
            return
        elif c_address != address and address in userlist:
            for u in userlist:
                if address == u:
                    address = userlist[userlist.index(u)-1]
                    faddress = True
                    continue
        else:
            client_socket.send(f"No client found at {address}\n".encode())


def mute(mname: str):
    muted.append(mname)

def unmute(mname: str):
    muted.remove(mname)
    try:
        tplaceholdertmp.cancel()
    except:
        pass

def tempmute(mname: str, time: int):
    global tplaceholdertmp
    mute(mname)
    #threading.Timer(time*60, unmute(name))
    tplaceholdertmp = threading.Timer(time*60, unmute, mname).start()

def verifyaccount(vname: str, vpasswd: str):
    vpasswdhash = hash(vpasswd)
    del vpasswd
    with open(f"{HOME}/scripts/store/chat-server/accountdb.plaindb", r) as db:
        while line := db.readline():
            verificationcontent = line.rstrip()



def createaccount(cname: str, cpasswd: str):
    cpasswdhash = hash(cpasswd)
    del cpasswd
    with open(f"{HOME}/scripts/store/chat-server/accountdb.plaindb", "a") as db:
        db.writelines(f"{cname}:{cpasswdhash}")
    print(f"Successfully created account with username: {cname} and passwordhash of {cpasswdhash}")


def handle_client(client_socket, client_address):
    print(f'New connection from {client_address}')
    if client_address[0] in [c.getpeername()[0] for c in clients]:
        print(f'{client_address} is attempting to connect twice')
        client_socket.send("Your already connected\n".encode())
        client_socket.close()
        return
    clients.append(client_socket)
    userlist.append(client_address[0])
    client_socket.send("Name: ".encode())
    try:
        name = client_socket.recv(1024).decode().replace("\n", "")
        if name:
            if name == "" or " " in name or SwearOrNot(name):
                client_socket.send("Your name have problems\n".encode())
                print(f"{client_address}'s name contains problem")
                clients.remove(client_socket)
                client_socket.close()
                return
            for n in names:
                if n == name:
                    snames = " ".join(names)
                    client_socket.send(f"Username taken, use an unused name, current names: {snames}\n".encode())
                    print(f'{client_address} is trying to use a taken name - {name}')
                    clients.remove(client_socket)
                    client_socket.close()
                    return
            userlist.insert(userlist.index(client_address[0])+1, name)
            print(userlist)
            print(userlist[userlist.index(name)])
        else:
            raise Exception(f"Client '{client_address}' disconnected without registering")
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
    with open(f"{HOME}/scripts/store/chat-server/chat.log", "r") as log:
        log_contents = log.read()
        client_socket.send(log_contents.encode())
    broadcast(f'{name} has joined the chat\n'.encode())
    #with open("chat.log", "r") as log:
    #    log_contents = log.read()
    #    client_socket.send(log_contents.encode())

    while True:
        try:
            message = client_socket.recv(1024)
            if message and name not in muted:
                rmsg = message.decode()
                if rmsg.startswith("$"): #or client_address[0] == "192.168.1.16" and rmsg.startswith("$"):
                    command = rmsg[1:].strip()
                    #command_parts = message[1:].strip().split()
                    print(f"Command spawned by {client_address},({name}): {command}")
                    if command == "list":
                        client_socket.send("Connected clients:\n".encode())
                        for c in clients:
                            c_address = c.getpeername()[0]
                            client_socket.send(f"{c_address}\n".encode())
                    if client_address[0] == "127.0.0.1":
                        if "kick" in command and len(command.split()) == 2:
                            kick_address = command.split()[1]
                            if kick_address != "127.0.0.1": # and client_address[0] != "192.168.1.16":
                                kick(kick_address, userlist[userlist.index(kick_address)+1], client_socket)
                        elif "mute" in command and len(command.split()) == 2:
                            mute_name = command.split()[1]
                            if mute_name != "127.0.0.1":
                                mute(mute_name)
                        elif "unmute" in command and len(command.split()) == 2:
                            unmute_name = command.split()[1]
                            if unmute_name in muted:
                                print(unmute_name)
                                unmute(unmute_name)
                        elif command not in cmdlist:
                            client_socket.send("Unknown command or syntax \n".encode())
                            print(f"{client_address},({name}) send an unknown command: {command}")
                if message.decode() != "" or message.decode() != " ":
                    #broadcast(f'{client_address}: {message.decode()}'.encode())
                    if SwearOrNot(message.decode()) and client_address[0] != "127.0.0.1":
                        broadcast(f'{name}: {len(message.decode())*"#"}\n'.encode())
                        if name not in sweart1 and name not in sweart2 and client_address[0] != "127.0.0.1":
                            sweart1.append(name)
                            print("t1")
                        elif name not in sweart2 and name in sweart1 and client_address[0] != "127.0.0.1":
                            sweart2.append(name)
                            sweart1.remove(name)
                            print("t2")
                        elif name not in sweart1 and name in sweart2 and client_address[0] != "127.0.0.1":
                            tempmute(name, 15)
                            print("tmpmute")
                    else:
                        broadcast(f'{name}: {message.decode()}'.encode())
                    print(f'{client_address}({name}): {message.decode()}')
            elif message and name in muted:
                client_socket.send("You have been muted\n".encode())
            else:
                raise Exception(f'Client {name} disconnected')
        except Exception as e:
            print(e)
            clients.remove(client_socket)
            print(f'{client_address} has left the chat')
            #broadcast(f'{client_address} has left the chat\n'.encode())
            broadcast(f'{name} has left the chat\n'.encode())
            client_socket.close()
            names.remove(name)
            userlist.remove(name)
            userlist.remove(client_address[0])
            try:
                muted.remove(name)
            except ValueError:
                pass
            return

def SwearOrNot(var):
    if any(word in var for word in swearList):
        return True
    else:
        return False


def accept_connections():
    global client_thread
    while True:
        try:
            client_socket, client_address = server_socket.accept()
            client_thread = threading.Thread(target=handle_client, args=(client_socket, client_address))
            client_thread.start()
        except KeyboardInterrupt:
            print("Closing server...")
            #broadcast("Closing server...")
            #client_thread.join()
            exit()

try:
    if __name__ == '__main__':
        #with open("chat.log", "w") as log:
        #    log.truncate(0)
        with open(f"{HOME}/scripts/store/chat-server/chat.log", "w") as log:
            log.truncate(0)
        try:
            server_thread = threading.Thread(target=accept_connections)
            server_thread.start()
            #client_socket, client_address = server_socket.accept()
            #handle_client(client_socket, client_address)
        except KeyboardInterrupt:
            print("Closing server...")
            #broadcast("Closing server...")
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
                    #broadcast("Closing server...")
                    #client_thread.join()
                    exit()
        except KeyboardInterrupt:
            print("Closing server...")
            #broadcast("Closing server...")
            #client_thread.join()
            exit()
except KeyboardInterrupt:
    print("Closing server...")
    #broadcast("Closing server...")
    #client_thread.join()
    exit()
