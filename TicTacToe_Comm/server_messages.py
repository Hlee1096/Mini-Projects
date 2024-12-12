from enum import Enum


class Server_Messages(Enum):
    LOGIN = "LOGIN"
    REGISTER = "REGISTER"
    ROOMLIST = "ROOMLIST"
    BADAUTH = "BADAUTH"
    NOROOM = "NOROOM"
    BEGIN = "BEGIN"
    INPROGRESS = "INPROGRESS"
    BOARDSTATUS = "BOARDSTATUS"
    GAMEEND = "GAMEEND"
    CREATE = "CREATE"
    JOIN = "JOIN"


def format_message(message_type, *args):
    return f"{message_type.value}:" + ":".join(args)


def format_bad_auth(message_type):
    return f"{message_type.value}"

# Example usage
# login_message = format_message(Server_Messages.LOGIN, "username", "password")
# register_message = format_message(Server_Messages.REGISTER, "username", "password")
# roomlist_message = format_message(Server_Messages.ROOMLIST, "mode")

# print(login_message)     # Outputs: LOGIN:username:password
# print(register_message)  # Outputs: REGISTER:username:password
# print(roomlist_message)  # Outputs: ROOMLIST:mode
