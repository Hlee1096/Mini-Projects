import socket
import client_messages
import sys
import threading
import game
from server_config_verification import printstd


def interpret_message(server_message: str, client_message: str, active_modes: dict, player_names: list, client_sided_board: list, current_username, read_sockets):
    server_message_list = server_message.split(":")
    client_message_list = client_message.split(":")
    protocol_name = server_message_list[0]

    if protocol_name == "LOGIN":
        # server_message_list[-1], is status code
        return login_message(server_message_list[-1], client_message_list[1], current_username)

    if protocol_name == "REGISTER":
        return register_message(server_message_list[-1], client_message_list[1])

    if protocol_name == "CREATE":
        return create_message(server_message_list[-1], client_message_list[1], active_modes)

    if protocol_name == "BADAUTH":
        return bad_auth_message()

    if protocol_name == "ROOMLIST":
        return roomlist_message(server_message_list, client_message_list[-1])

    if protocol_name == "JOIN":
        return join_message(server_message_list[-1], client_message_list, active_modes)

    if protocol_name == "BEGIN":
        return begin_message(server_message_list, active_modes, player_names)

    if protocol_name == "INPROGRESS":
        return inprogress_message(server_message_list, active_modes, read_sockets)

    if protocol_name == "BOARDSTATUS":
        return board_state_message(server_message_list, active_modes, player_names, client_sided_board)

    if protocol_name == "GAMEEND":
        game_end_message(server_message_list, active_modes,
                         client_sided_board, current_username, player_names, read_sockets)


def game_end_message(message_list, active_modes, client_sided_board, current_username, player_names, read_sockets):
    status_code = message_list[2]
    if len(message_list) > 3:
        winner_username = message_list[3]
    board_state = message_list[1]
    letter_index = 0
    for i in range(3):
        for j in range(3):
            number = board_state[letter_index]
            if number == "0":
                pass
            if number == "1":
                client_sided_board[i][j] = 'X'
            if number == "2":
                client_sided_board[i][j] = 'O'
            letter_index += 1
    game.print_board(client_sided_board)
    read_sockets.add(sys.stdin)
    #print(active_modes["playing"])
    if status_code == "1":
        print("Game ended in a draw")
    elif status_code == "0":

        if current_username[0] == winner_username:
            print("Congratulations, you won!")
        elif active_modes["playing"] is True :
            print("Sorry you lost. Good luck next time.")
        else:
            print(f"{winner_username} has won this game")
    
    elif status_code == "2":
        print(f"{winner_username} won due to the opposing player forfeiting")

    client_sided_board = game.create_board()
    active_modes["your_turn"] = False
    active_modes["playing"] = False
    active_modes["watching"] = False
    player_names = []


def board_state_message(message_list, active_modes, player_names, client_sided_board):
    board_state = message_list[1]
    letter_index = 0
    for i in range(3):
        for j in range(3):
            number = board_state[letter_index]
            if number == "0":
                pass
            if number == "1":
                client_sided_board[i][j] = 'X'
            if number == "2":
                client_sided_board[i][j] = 'O'
            letter_index += 1
    game.print_board(client_sided_board)

    if active_modes["your_turn"] is True and active_modes["playing"] is True:
        active_modes["your_turn"] = False
    elif active_modes["your_turn"] is False and active_modes["playing"] is True:
        active_modes["your_turn"] = True

    return


def inprogress_message(message_list, active_modes, read_sockets):
    current_turn = message_list[1]
    opposing = message_list[2]
    active_modes["watching"] = True
    read_sockets.remove(sys.stdin)
    print(
        f"Match between {current_turn} and {opposing} is currently in progress, it is {current_turn}'s turn")
    return


def begin_message(message_list: list, active_modes: dict, player_names: list):
    playing_flag = active_modes["playing"]
    your_turn_flag = active_modes["your_turn"]
    player1 = message_list[1]
    player2 = message_list[2]
    player_names.append(player2)
    if playing_flag and your_turn_flag:
        print("place your first marker")
        return
    if playing_flag and not your_turn_flag:
        print(f"Please wait until {player1} has placed their marker")
        return

    print(
        f"match between {player1} and {player2} will commence, it is currently {player1}'s turn.")


def execute_command(client_socket: socket, user_input: list, client_sided_board: list, active_modes: dict):
    if len(user_input) == 0:
        print("Unknown command: <unknown command name>")
        return None

    if user_input[0] == "QUIT":
        client_socket.close()
        sys.exit()

    if user_input[0] == "LOGIN":
        username, password = login_register_sequence()
        message_packet = client_messages.format_message(
            client_messages.Client_Messages.LOGIN, username, password)

    elif user_input[0] == "REGISTER":
        username, password = login_register_sequence()
        message_packet = client_messages.format_message(
            client_messages.Client_Messages.REGISTER, username, password)

    elif user_input[0] == "CREATE":
        room_name = create_sequence()
        message_packet = client_messages.format_message(
            client_messages.Client_Messages.CREATE, room_name)

    elif user_input[0] == "ROOMLIST":
        player_mode = roomlist_sequence()
        message_packet = client_messages.format_message(
            client_messages.Client_Messages.ROOMLIST, player_mode)

    elif user_input[0] == "JOIN":
        room_name, player_mode = join_sequence()
        message_packet = client_messages.format_message(
            client_messages.Client_Messages.JOIN, room_name, player_mode)

    elif user_input[0] == "PLACE":
        x, y = place_sequence(client_sided_board, active_modes)
        message_packet = client_messages.format_message(
            client_messages.Client_Messages.PLACE, str(x), str(y))
    elif user_input[0] == "FORFEIT":
        message_packet = client_messages.format_message_non_arg(
            client_messages.Client_Messages.FORFEIT)
    else:
        print("Unknown command: <unknown command name>")
        return None

    return message_packet


def place_sequence(board: list, active_modes: dict):
    while True:
        while True:
            try:
                x = int(input("Column: "))
                if x > 2 or x < 0:
                    printstd("Column values must be an integer between 0 and 2")
                else:
                    break
            except ValueError:
                print("Column values must be an integer between 0 and 2")

        while True:
            try:
                y = int(input("Row: "))
                if y > 2 or y < 0:
                    print("Row values must be an integer between 0 and 2")
                else:
                    break
            except ValueError:
                print("Row values must be an integer between 0 and 2")

        if (occupant := board[y][x]) == game.EMPTY:
            active_modes["your_turn"] = False
            return (x, y)
        print(f"({x+1}, {y+1}) is occupied by {occupant}")


def join_sequence():
    room_name = input("Enter room name you want to join: ")
    while True:
        player_mode = input("You wish to join the room as: (Player/Viewer) ")
        player_mode = player_mode.upper()
        if player_mode == "PLAYER" or player_mode == "VIEWER":
            break
        else:
            print("Unknown input.")
    return room_name, player_mode


def join_message(status_code: str, client_message_list: list, active_modes: dict):
    if status_code == "3":
        printstd("Wrong args")
        return
    room_name = client_message_list[1]
    player_mode = client_message_list[2]

    if status_code == "0":
        print(
            f"Successfully joined room {room_name} as a {player_mode.lower()}")

        if (player_mode.lower()) == "player":
            active_modes["playing"] = True
            active_modes["your_turn"] = False

        if (player_mode.lower()) == "viewer":
            active_modes["playing"] = False
            active_modes["your_turn"] = False
        return False

    if status_code == "1":
        printstd(f"Error: No room named {room_name}")
        return

    if status_code == "2":
        printstd(f"Error: The room {room_name} already has 2 players")
        return


def wait_sequence():
    print("Waiting for other player...")


def roomlist_message(message_list: list, player_mode: str):
    status_code = message_list[2]
    if status_code == "1":
        printstd("Error: Please input a valid mode.")
        return
    print(f"Room available to join as {player_mode}: {message_list[3]} ")


def bad_auth_message():
    printstd("Error: You must be logged in to perform this action")
    return


def register_message(status_code: str, username: str):
    if status_code == "0":
        print(f"Successfully created user account {username}")
        return
    elif status_code == "1":
        printstd(f"Error: User {username} already exists")
        return
    elif status_code == "2":
        printstd("Incorrect args")
        return


def login_message(status_code: str, username: str, current_username) -> None:
    if status_code == "0":
        print(f"Welcome {username}")
        current_username.append(username)
        return
    if status_code == "1":
        printstd(f"Error: User {username} not found")
        return
    elif status_code == "2":
        printstd(f"Error: Wrong password for user {username}")
        return
    else:
        printstd("Incorrect args")
        return


def login_register_sequence() -> str:
    username = input("Enter username: ")
    password = input("Enter password: ")
    return username, password


def create_sequence() -> str:
    room_name = input("Enter room name you want to create: ")
    return room_name


def create_message(status_code: str, room_name: str, active_modes: dict) -> str:
    if status_code == "0":
        print(f"Successfully created room {room_name}")
        print("Waiting for other player....")
        active_modes["playing"] = True
        active_modes["your_turn"] = True
        return
    if status_code == "1":
        printstd(f"Error: Room {room_name}is invalid")
        return
    if status_code == "2":
        printstd(f"Error: Room {room_name} already exists")
        return
    if status_code == "3":
        printstd("Error: Server already contains a maximum of 256")
        return
    if status_code == "4":
        printstd("Incorrect args")
        return


def roomlist_sequence():
    player_mode = input(
        "Do you want to have a room list as player or viewer? (Player/Viewer) ")
    return player_mode.upper()
