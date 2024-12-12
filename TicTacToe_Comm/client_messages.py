from enum import Enum


class Client_Messages(Enum):
    LOGIN = "LOGIN"
    REGISTER = "REGISTER"
    ROOMLIST = "ROOMLIST"
    CREATE = "CREATE"
    JOIN = "JOIN"
    PLACE = "PLACE"
    FORFEIT = "FORFEIT"


def format_message(message_type, *args):

    return f"{message_type.value}:" + ":".join(args)


def format_message_non_arg(message_type):

    return f"{message_type.value}"
# # Example usage
# login_message = format_message(Client_Messages.LOGIN, "username", "password")
# register_message = format_message(Client_Messages.REGISTER, "username", "password")
# roomlist_message = format_message(Client_Messages.ROOMLIST, "mode")

# print(login_message)     # Outputs: LOGIN:username:password
# print(register_message)  # Outputs: REGISTER:username:password
# print(roomlist_message)  # Outputs: ROOMLIST:mode
