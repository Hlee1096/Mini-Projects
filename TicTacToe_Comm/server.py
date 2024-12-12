import sys
import socket
import select
import os
import server_messages
import room


from server_config_verification import printstd, validate_server_config
from user_data_verification import validate_user_config
from server_execution import *


HOST = "127.0.0.1"


def server_loop(server_sock: socket.socket, user_database: str) -> None:
    read_socks = {server_sock}
    write_socks = set()

    # dictionary which will hold the socket and a username to see if authenticated
    users = {}

    # Will hold socket and if it is valid for them to recieve messages
    appropriate_sockets = {}

    # Will hold a dictionary , socket and message as a list
    message_buffer = {}

    # associates sockets with rooms.
    users_room = {}

    # List to hold room objects
    rooms = []
    with server_sock:
        while True:

            select_read_socks, select_write_socks, select_except_socks = select.select(
                read_socks,
                write_socks,  # empty tuple
                (),
            )

            # check_rooms(rooms, users).

            for sock in select_read_socks:
                if sock is server_sock:
                    client_sock, client_addr = sock.accept()
                    client_sock.setblocking(False)
                    read_socks.add(client_sock)
                    # print("Connection from", client_addr)
                    users[client_sock] = None
                    appropriate_sockets[client_sock] = True
                    users_room[client_sock] = None
                    message_buffer[client_sock] = []
                    print(read_socks)
                    # print(users)
                    # print(appropriate_sockets)
                else:
                    # `sock` is client socket
                    client_msg = sock.recv(8192)
                    return_message = "THIS SHOULD NEVER BE SENT"
                    begin_message_flag = False
                    in_progess_flag = False

                    # print(client_msg.decode())
                    raw_client_msg = client_msg.decode().strip().split(":")
                    print(raw_client_msg)
                    # print(users_room)
                    if not client_msg:
                        # Empty message (0 bytes) indicates disconnection
                        print("disconnected")
                        disconnected_sequence(
                            read_socks, write_socks, rooms, sock, users, message_buffer, appropriate_sockets, users_room)
                        continue
                    else:
                        if appropriate_sockets[sock] is False:
                            message_buffer[sock].append(raw_client_msg)
                            continue
                        if len(message_buffer[sock]) > 0 and appropriate_sockets[sock] is True:
                            raw_client_msg = message_buffer[sock].pop(0)

                        return_message, begin_message_flag, in_progess_flag = execute_commands(
                            raw_client_msg, user_database, users, rooms, sock, appropriate_sockets, users_room)

                        print(return_message)
                        sock.send((return_message + "\n").encode())

                        if in_progess_flag is True:
                            in_progess_flag = False
                            send_in_progress(rooms, sock, users_room, users)

                        if begin_message_flag is True:
                            begin_message_flag = False
                            running_room = send_begin_message(
                                rooms, users, raw_client_msg[1], appropriate_sockets)

                        if users_room[sock] is not None:
                            targeted_room = users_room[sock]
                            if targeted_room.has_started():
                                queue_message_exec(
                                    users_room, sock, message_buffer, appropriate_sockets, user_database, users, rooms)

                for sock in select_write_socks:
                    pass
                    # sock.send("ignore this".encode())


def main(args: list[str]) -> None:

    if len(args) != 1:
        printstd("Error: Expecting 1 argument: <server config path>.")
        sys.exit()
    PORT, user_database = validate_server_config(args[0])
    validate_user_config(user_database)
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    server_socket.bind((HOST, PORT))
    server_socket.setblocking(False)
    server_socket.listen()
    server_loop(server_socket, user_database)


if __name__ == "__main__":
    main(sys.argv[1:])
