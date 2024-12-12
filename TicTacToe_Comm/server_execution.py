import re
import json
import bcrypt
import socket
import game
import threading
import room as game_room
import server_messages


def disconnected_sequence(read_socks: list, write_socks: list, rooms: list, sock: socket, users: dict, message_buffer: dict, appropriate_sockets: dict, users_room):
    read_socks.remove(sock)
    try:
        write_socks.remove(sock)
    except KeyError:
        pass

    room_to_be_deleted = find_room(users_room, sock)
    print(read_socks)
    if room_to_be_deleted is None:
        del message_buffer[sock]
        del appropriate_sockets[sock]
        del users_room[sock]
        sock.close()
        return

    if room_to_be_deleted is not None:
        socket_viewer_list = room_to_be_deleted.get_viewers()
        viewer_mode = False
        for viewer_sock in socket_viewer_list:
            if sock == viewer_sock:
                viewer_mode = True
                break
        if viewer_mode is True:
            del message_buffer[sock]
            del appropriate_sockets[sock]
            del users_room[sock]
            return

        socket_list = room_to_be_deleted.get_players_and_viewers()
        room_board = room_to_be_deleted.get_board()
        player1_sock = room_to_be_deleted.get_player1()
        player2_sock = room_to_be_deleted.get_player2()
        if player2_sock is None:
            del message_buffer[sock]
            del appropriate_sockets[sock]
            del users_room[sock]
            rooms.remove(room_to_be_deleted)
            return

        if sock == player1_sock:
            winner_sock = player2_sock
        else:
            winner_sock = player1_sock
        winner_name = users[winner_sock]
        board_state = board_status(room_board)
        if room_to_be_deleted.is_ended() is False:
            message = server_messages.format_message(
                server_messages.Server_Messages.GAMEEND, board_state, "2", winner_name)
            if len(socket_list) > 0:
                send_message_player_sockets(socket_list, sock, message)
        try:
            rooms.remove(room_to_be_deleted)
        except ValueError:
            pass
        del message_buffer[sock]
        del appropriate_sockets[sock]
        del users_room[sock]


def execute_commands(raw_client_msg: list, user_database: str, users: dict, rooms: list, sock: socket, appropriate_sockets: dict, users_room: dict):
    begin_message_flag = False
    in_progress_flag = False
    return_message = ""
    if raw_client_msg[0] == "LOGIN":
        status_code = authenticate_user(
            raw_client_msg, user_database)
        if status_code == 0:
            users[sock] = raw_client_msg[1]
            # print(users)
        status_code = str(status_code)
        return_message = server_messages.format_message(
            server_messages.Server_Messages.LOGIN, "ACKSTATUS", status_code)

    elif raw_client_msg[0] == "REGISTER":
        status_code = register_user(
            raw_client_msg, user_database)
        status_code = str(status_code)
        return_message = server_messages.format_message(
            server_messages.Server_Messages.REGISTER, "ACKSTATUS", status_code)

    elif raw_client_msg[0] == "CREATE":
        if users[sock] is None:
            return_message = server_messages.format_bad_auth(
                server_messages.Server_Messages.BADAUTH)
        else:
            status_code = verify_room_name(
                raw_client_msg, rooms)
            if status_code == 0:
                status_code = str(status_code)
                room_name = raw_client_msg[1]
                # write_socks.add(sock)
                targeted_room = create_room(room_name, rooms, sock)
                appropriate_sockets[sock] = False
                users_room[sock] = targeted_room

                for item in rooms:
                    print(item)

                return_message = server_messages.format_message(
                    server_messages.Server_Messages.CREATE, "ACKSTATUS", status_code)

            else:
                status_code = str(status_code)
                return_message = server_messages.format_message(
                    server_messages.Server_Messages.CREATE, "ACKSTATUS", status_code)

    elif raw_client_msg[0] == "ROOMLIST":
        if users[sock] is None:
            return_message = server_messages.format_bad_auth(
                server_messages.Server_Messages.BADAUTH)

        else:
            status_code = roomlist_exec(raw_client_msg)

            if status_code == 0:
                status_code = str(status_code)
                room_name_list = []
                room_name_message = ""
                for room_obj in rooms:
                    room_name_list.append(room_obj.get_name())
                    print(room_obj.get_name())
                room_name_list.sort()
                room_name_message = ", ".join(room_name_list)
                return_message = server_messages.format_message(
                    server_messages.Server_Messages.ROOMLIST, "ACKSTATUS", status_code, room_name_message)
            else:
                status_code = str(status_code)
                return_message = server_messages.format_message(
                    server_messages.Server_Messages.ROOMLIST, "ACKSTATUS", status_code)
    elif raw_client_msg[0] == "JOIN":
        if users[sock] is None:
            return_message = server_messages.format_bad_auth(
                server_messages.Server_Messages.BADAUTH)
        else:
            status_code, targeted_room, in_progress_flag = join_exec(
                raw_client_msg, rooms, sock)
            if status_code == 0:
                users_room[sock] = targeted_room
            status_code = str(status_code)
            return_message = server_messages.format_message(
                server_messages.Server_Messages.JOIN, "ACKSTATUS", status_code)

            if status_code == "0" and raw_client_msg[2] == "PLAYER":
                begin_message_flag = True
                appropriate_sockets[sock] = False

            if status_code == "0" and raw_client_msg[2] == "VIEWER":
                appropriate_sockets[sock] = False
    elif raw_client_msg[0] == "PLACE":
        if users[sock] is None:
            return_message = server_messages.format_bad_auth(
                server_messages.Server_Messages.BADAUTH)
        elif users_room[sock] is None:
            return_message = server_messages.format_bad_auth(
                server_messages.Server_Messages.NOROOM)
        else:
            return_message = place_exec(
                raw_client_msg, users_room, sock, users, appropriate_sockets)
    elif raw_client_msg[0] == "FORFEIT":
        if users[sock] is None:
            return_message = server_messages.format_bad_auth(
                server_messages.Server_Messages.BADAUTH)
        elif users_room[sock] is None:
            return_message = server_messages.format_bad_auth(
                server_messages.Server_Messages.NOROOM)
        else:
            return_message = forfeit_exec(
                sock, users_room, appropriate_sockets, users)
    return return_message, begin_message_flag, in_progress_flag


def forfeit_exec(sock, users_room, appropriate_sockets, users):
    status_code = '2'
    targeted_room = users_room[sock]
    targeted_room_board = targeted_room.get_board()
    board_state = board_status(targeted_room_board)

    player1_sock = targeted_room.get_player1()
    player2_sock = targeted_room.get_player2()
    if sock == player1_sock:
        winner = player2_sock
    else:
        winner = player1_sock

    winner_username = find_key(users, winner)
    player_sockets = targeted_room.get_players_and_viewers()
    message = server_messages.format_message(
        server_messages.Server_Messages.GAMEEND, board_state, status_code, winner_username)
    send_message_player_sockets(player_sockets, sock, message)
    targeted_room.game_end()
    return message


def place_exec(args: list, users_room: dict, sock: socket, users, appropriate_sockets):
    x_coord = int(args[1])
    y_coord = int(args[2])
    targeted_room = users_room[sock]
    icon = targeted_room.get_icon()
    targeted_room_board = targeted_room.get_board()
    player_sockets = targeted_room.get_players_and_viewers()
    game.my_player_turn(icon, targeted_room_board, x_coord, y_coord)
    player1_sock = targeted_room.get_player1()
    player2_sock = targeted_room.get_player2()
    if sock == player1_sock:
        other_player = player2_sock
    else:
        other_player = player1_sock
    appropriate_sockets[sock] = False
    appropriate_sockets[other_player] = True

    board_state = board_status(targeted_room_board)

    if check_victory_loss_draw(targeted_room_board, icon) is not False:
        game_state = check_victory_loss_draw(targeted_room_board, icon)
        targeted_room.game_end()
        if game_state == "Win":
            status_code = '0'
            winner = users[sock]
            message = server_messages.format_message(
                server_messages.Server_Messages.GAMEEND, board_state, status_code,winner)
            send_message_player_sockets(player_sockets, sock, message)
            return message
        elif game_state == "Draw":
            status_code = '1'
            message = server_messages.format_message(
                server_messages.Server_Messages.GAMEEND, board_state, status_code)
            send_message_player_sockets(player_sockets, sock, message)
            return message

    targeted_room.change_turn()
    message = server_messages.format_message(
        server_messages.Server_Messages.BOARDSTATUS, board_state)
    send_message_player_sockets(player_sockets, sock, message)
    return message


def send_message_player_sockets(socket_list, sock, message):
    for game_socket in socket_list:
        if game_socket == sock:
            continue
        else:
            game_socket.send((message + "\n").encode())


def board_status(board: list):
    board_state = ""
    for row in board:
        for item in row:
            if item == game.EMPTY:
                board_state = board_state + "0"
                continue
            if item == game.CROSS:
                board_state = board_state + "1"
                continue
            if item == game.NOUGHT:
                board_state = board_state + "2"

    return board_state


def check_victory_loss_draw(board: list, icon):
    if game.player_wins(icon, board):
        return "Win"
    elif game.players_draw(board):
        return "Draw"
    return False


def authenticate_user(args: list, user_database: str) -> int:
    if len(args) != 3:
        return 3  # Incorrect Arguments
    username = args[1]
    password = args[2].encode()
    with open(user_database, 'r') as file_obj:
        database_data = json.load(file_obj)

        for record in database_data:
            if username == record["username"]:
                if bcrypt.checkpw(password, record["password"].encode()):
                    return 0  # Successful

                else:
                    return 2  # Wrong Pass

    return 1  # No username


def register_user(args: list, user_database: str) -> int:

    # https://stackoverflow.com/questions/12994442/how-to-append-data-to-a-json-file by user nneonneo

    if len(args) != 3:
        return 2  # Incorrect Arguments
    username = args[1]
    password = args[2].encode()
    hashed_password = bcrypt.hashpw(password, bcrypt.gensalt())
    with open(user_database, 'r') as file_obj_read:
        original_data = json.load(file_obj_read)

        for record in original_data:
            if username == record["username"]:
                return 1
    with open(user_database, 'w') as file_obj:
        # json.dump([], file_obj)
        new_entry = {"username": username,
                     "password": hashed_password.decode()}
        original_data.append(new_entry)
        json.dump(original_data, file_obj)
    return 0


def verify_room_name(args: list, rooms: list) -> int:
    if len(args) != 2:
        return 4

    room_name = args[1]
    illegal_chars = re.findall("[^a-zA-Z0-9_ -]", room_name)
    if len(illegal_chars) > 0 or len(room_name) > 20:
        return 1

    for room in rooms:
        if room_name == room.get_name():
            return 2

    if len(rooms) >= 256:
        return 3

    return 0


def roomlist_exec(args: list) -> int:
    if len(args) != 2:
        return 1
    player_mode = args[1].upper()

    if player_mode != "PLAYER" and player_mode != "VIEWER":
        return 1

    return 0


def create_room(room_name: str, rooms: list, sock):
    room_id = len(rooms) + 1
    room = game_room.Room(room_name, room_id)
    rooms.append(room)
    room.add_player1(sock)
    return room


def join_exec(args: list, rooms: list, sock) -> int:
    targeted_room = None
    in_progress_flag = False

    if len(args) != 3:
        return 3, targeted_room, in_progress_flag

    room_name = args[1]
    player_mode = args[2]

    if (player_mode != "VIEWER") and (player_mode != "PLAYER"):
        return 3, targeted_room, in_progress_flag

    room_found = False

    for room in rooms:
        if room_name == room.get_name():
            room_found = True
            targeted_room = room
            break

    if not room_found:
        return 1, targeted_room, in_progress_flag

    if player_mode == "PLAYER":
        if not (targeted_room.can_add_players()):
            return 2, targeted_room, in_progress_flag
        else:
            targeted_room.add_player2(sock)
            return 0, targeted_room, in_progress_flag

    if player_mode == "VIEWER":
        targeted_room.add_viewer(sock)
        if targeted_room.has_started() is True:
            in_progress_flag = True
        return 0, targeted_room, in_progress_flag


def find_key(input_dict, value):
    # https://stackoverflow.com/questions/67351965/use-python-function-to-get-key-by-value-in-dictionary-and-return-either-key-or
    # For providing this function to get key from value

    for key, val in input_dict.items():
        if key == value:
            return val
    return "None"


def check_rooms(rooms: list, users: dict):
    for room in rooms:
        if room.is_read.is_ready_to_start():
            for users in room.get_players_and_viewers():
                print(users)


def find_room(user_rooms: dict, sock):
    return user_rooms[sock]


def wait_sequence():
    print("waiting for a second")


def thread_wait():
    t = threading.Timer(1.0, wait_sequence)
    t.start()


def send_in_progress(rooms: list, sock: socket, users_room: dict, users: dict):
    targeted_room = users_room[sock]
    current_player_turn = targeted_room.get_player_turn()
    opposing_player_turn = targeted_room.get_opposing_player()
    current_player_turn_name = find_key(users, current_player_turn)
    opposing_player_turn_name = find_key(users, opposing_player_turn)
    message = server_messages.format_message(
        server_messages.Server_Messages.INPROGRESS, current_player_turn_name, opposing_player_turn_name)
    thread_wait()
    sock.send((message + "\n").encode())
    return


def queue_message_exec(users_room: dict, sock, message_buffer, appropriate_sockets, user_database, users, rooms):
    targeted_room = users_room[sock]
    if targeted_room is None:
        return
    other_player_socket = None
    player1_sock = targeted_room.get_player1()
    player2_sock = targeted_room.get_player2()

    if sock == player1_sock:
        other_player_socket = player2_sock
    else:
        other_player_socket = player1_sock

    if len(message_buffer[other_player_socket]) > 0 and appropriate_sockets[other_player_socket] is True:
        raw_client_msg = message_buffer[other_player_socket].pop(
            0)

        return_message, begin_message_flag, in_progess_flag = execute_commands(
            raw_client_msg, user_database, users, rooms, other_player_socket, appropriate_sockets, users_room)

        other_player_socket.send(
            (return_message + "\n").encode())


def send_begin_message(room_list: list, user: dict, room_name: str, appropriate_sockets: dict):
    targeted_room = None
    for room in room_list:
        if room.get_name() == room_name:
            targeted_room = room
            break
    player_viewer_list = targeted_room.get_players_and_viewers()
    player1, player2 = targeted_room.get_player1_and_player2()
    targeted_room.start()
    appropriate_sockets[player1] = True
    player1_name = find_key(user, player1)
    player2_name = find_key(user, player2)

    message = server_messages.format_message(
        server_messages.Server_Messages.BEGIN, player1_name, player2_name)
    for cli_socket in player_viewer_list:
        cli_socket.send((message + "\n").encode())
    return targeted_room


def main():
    pass


if __name__ == "__main__":
    main()
