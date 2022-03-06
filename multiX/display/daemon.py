#!/usr/bin/python3
import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk
import sys
import os
import socket
from select import select
from time import sleep

from multix import perr, dbgp
from multixConfig import settings
from multixDefines import enums, proto
import multixGUI

class multixServer():

    def __init__(self, socketPath):

        self.socketPath = socketPath

        if os.path.exists(self.socketPath):
            dbgp("Trying to reclaim the socket path, because it already exists")
            os.remove(self.socketPath)

        self.sockfd = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sockfd.bind(self.socketPath)
        self.sockfd.listen(1)

        self.client, self.clientAddress = self.sockfd.accept()

    def canRead(self):

        canReadValue = False

        if len(select([self.client], [], [], 0)[0]) > 0:
            canReadValue = True

        return canReadValue

    def recvCommand(self):

        msg = self.client.recv(1024).strip(b"\x00").decode()
        if len(msg) == 0:
            #the other side disconnected; terminate the server
            perr("The terminal side disconnected from the socket. Quitting")
            daemonQuit(0)
        dbgp(f"Server received data: {msg}")

        command = self._parseCmd(msg)

        dbgp(f"command = {command}")
        return command

    def send(self, data):

        dbgp(f"Server sending data: {data}")

        self.client.send(data.encode())

    def _parseCmd(self, data):

        cmd = list()

        splitData = data.partition(" ")

        try:
            cmd.append(data.partition(" ")[0])
            cmd.append(data.partition(" ")[2])
        except IndexError:
            return None

        return cmd

    def __del__(self):
        self.sockfd.close()
        if os.path.exists(self.socketPath):
            os.remove(self.socketPath)

def daemonQuit(arg):
    server.__del__()
    Gtk.main_quit()
    sys.exit(0)

#-=-=-=Main daemon + GUI loop=-=-=-

if __name__ == "__main__":
    try:
    
        server = multixServer(settings.SOCKET_PATH)
        rootWin = multixGUI.displayWindow(server.send)
        rootWin.connect("destroy", daemonQuit)
        rootWin.show_all()
    
        while True:
            if server.canRead():
                command = server.recvCommand()
                if command:
                    com = command[0]
                    args = command[1]
                    dbgp(f"Processing a command: {com}; with args: {args}")
                    if com == proto.MSG_CWD:
                        rootWin.activePath = args
                        rootWin.mode = enums.DISPLAY_MODE_FM
                        rootWin.update()
                    else:
                        perr(f"Unimplemented command received: {com}")
                else:
                    perr("Malformed command received")
    
            if Gtk.events_pending():
                Gtk.main_iteration()
            else:
                sleep(0.05)

    except Exception as e:
        perr(f"Received an exception")
        print(e)
        server.__del__()
        Gtk.main_quit()
        sys.exit()
