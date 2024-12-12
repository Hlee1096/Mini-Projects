import sys
import socket
import select
import threading
import game
from server_config_verification import printstd
from client_exec import *


def main(args: list[str]) -> None:
    if len(args) != 2:
        printstd("Error: Expecting 2 arguments: <server address> <port>")
        sys.exit()

    host = args[0]
    port = int(args[1])
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        socket_set = {client_socket, sys.stdin}
        client_socket.connect((host, port))
    except ConnectionRefusedError:
        printstd("Error: cannot connect to server at <server address> and <port>.")
        sys.exit()

    active_modes = {"your_turn": False, "playing": False, "watching": False}
    player_names = []
    current_username = []
    client_sided_board = game.create_board()
    try:
        while True:
            read_sockets, write_sockets, error_sockets = select.select(
                socket_set, (), ())
            for sock in read_sockets:

                if sock == client_socket:
                    server_message = client_socket.recv(8192).decode().strip()
                    if not server_message:
                        print("DISCONNECTED!")
                        client_socket.close()
                        sys.exit()

                    print(server_message)

                    interpret_message(
                        server_message, message_packet, active_modes, player_names, client_sided_board, current_username, socket_set)
                else:
                    message_packet = ""
                    server_message = ""
                    try:
                        user_input = input("").split()
                    except EOFError:
                        if active_modes["watching"] is True:
                            pass
                        else:
                            sys.exit()
                    except KeyboardInterrupt:
                        if active_modes["watching"] is True:
                            pass
                        else:
                            sys.exit()

                    message_packet = execute_command(
                        client_socket, user_input, client_sided_board, active_modes)

                    if message_packet is None:
                        continue

                    client_socket.sendall((message_packet + "\n").encode())
    except EOFError:
        if active_modes["watching"] is True:
            pass
        else:
            sys.exit()
    except KeyboardInterrupt:
        print(active_modes["watching"])
        print(socket_set)


if __name__ == "__main__":
    main(sys.argv[1:])
