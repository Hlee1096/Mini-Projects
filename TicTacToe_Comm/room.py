
import game


class Room:
    def __init__(self, name, room_id):
        self.name = name
        self.room_id = room_id

        self.viewers = []
        self.player1 = None
        self.player2 = None

        self.board = game.create_board()
        self.started = False

        self.icon = game.CROSS

        self.player_turn = self.player1
        self.ready_to_start = False
        self.sent_start_message = False
        self.game_ended = False

    def is_ended(self):
        if self.game_ended is True:
            return True
        else:
            return False

    def game_end(self):
        self.game_ended = True

    def change_turn(self):
        if self.player_turn is self.player1:
            self.player_turn = self.player2
            self.icon = game.NOUGHT
        else:
            self.player_turn = self.player1
            self.icon = game.CROSS

    def has_started(self):
        return self.started

    def get_icon(self):
        return self.icon

    def start(self):
        if self.started is False:
            self.started = True

    def get_player_turn(self):
        return self.player_turn

    def get_opposing_player(self):
        if self.player_turn is self.player1:
            return self.player2
        else:
            return self.player1

    def get_player1(self):
        return self.player1

    def get_player2(self):
        return self.player2

    def get_board(self):
        return self.board

    def is_ready_to_start(self) -> bool:
        if self.player1 is not None and self.player2 is not None and self.sent_start_message is False:
            self.sent_start_message = True
            return True
        return False

    def get_viewers(self):
        return self.viewers

    def get_players_and_viewers(self) -> list:
        if self.player1 is not None and self.player2 is not None:
            player_viewer_list = self.viewers.copy()
            player_viewer_list.append(self.player1)
            player_viewer_list.append(self.player2)
            return player_viewer_list
        else:
            print("The game isn't ready")

    def get_player1_and_player2(self):
        return self.player1, self.player2

    def add_viewer(self, viewer):
        self.viewers.append(viewer)

    def get_players(self):
        print("viewers:")
        for player in self.viewers:
            print(player)

        print(f"players: {self.player1} and {self.player2}")

    def add_player1(self, player):
        if self.player1 is None:
            self.player1 = player
            self.player_turn = player
        else:
            print("You can't overwrite an existing player1")

    def add_player2(self, player):
        if self.player2 is None:
            self.player2 = player
        else:
            print("You can't overwrite an existing player2")

    def can_add_players(self) -> bool:
        if self.player1 is None or self.player2 is None:
            return True
        return False

    def get_name(self):
        return self.name

    def __str__(self):
        return f"room name:{self.name} and room id: {self.room_id}"
