# CSCI3180 Principles of Programming Languages
# --- Declaration ---
# I declare that the assignment here submitted is original except for source
# material explicitly acknowledged. I also acknowledge that I am aware of
# University policy and regulations on honesty in academic work, and of the
# disciplinary guidelines and procedures applicable to breaches of such policy
# and regulations, as contained in the website
# http://www.cuhk.edu.hk/policy/academichonesty/

# Assignment 2
# Name : GAO Ming Yuan
# Student ID : 1155107738
# Email Addr : 1155107738@link.cuhk.edu.hk

from utils import *
import time

class Board():
    def __init__(self):
        self.state = ['.' for i in range(16)]
        self.mills = [[0, 1, 2], [3, 4, 5], [10, 11, 12], [13, 14, 15], 
                      [0, 6, 13], [3, 7, 10], [5, 8, 12], [2, 9, 15]]
        # This parameter pre-defines all possible situations of mills on the board.
        self.edges = [[0, 1], [1, 2], 
                      [3, 4], [4, 5], 
                      [6, 7], [8, 9],
                      [10, 11], [11, 12], 
                      [13, 14], [14, 15], 
                      [0, 6], [6, 13], 
                      [3, 7], [7, 10], 
                      [1, 4], [11, 14],
                      [5, 8], [8, 12],
                      [2, 9], [9, 15]]
        # This parameter pre-defines all possible situations of connectivity of edges on the board.

    def print_board(self):
        def color(x):
            if x == '.': return x
            return g(x) if x == '#' else b(x)
        #print(self.state)
        print(color(self.state[0]), '-' * 9, color(self.state[1]), '-' * 9, color(self.state[2]), end='    ');
        print('a', '-' * 9, 'b', '-' * 9, 'c')
        print('|   ', ' ' * 6, '|', ' ' * 6, '   |', end='    ');
        
        print('|   ', ' ' * 6, '|', ' ' * 6, '   |')
        
        print('|   ', color(self.state[3]), '-' * 4, color(self.state[4]), '-' * 4, color(self.state[5]), '   |', end='    ');
        
        print('|   ', 'd', '-' * 4, 'e', '-' * 4, 'f', '   |')
        print('|   ' * 2, ' ' * 7, '   |' * 2, end='    ');
        print('|   ' * 2, ' ' * 7, '   |' * 2)
        
        print(color(self.state[6]), '-', color(self.state[7]), ' ' * 13, color(self.state[8]), '-', color(self.state[9]), end='    '); 
        print('g', '-', 'h', ' ' * 13, 'i', '-', 'j',)
        
        print('|   ' * 2, ' ' * 7, '   |' * 2, end='    '); 
        print('|   ' * 2, ' ' * 7, '   |' * 2)
        
        print('|   ', color(self.state[10]), '-' * 4, color(self.state[11]), '-' * 4, color(self.state[12]), '   |', end='    '); 
        print('|   ', 'k', '-' * 4, 'l', '-' * 4, 'm', '   |')
        
        print('|   ', ' ' * 6, '|', ' ' * 6, '   |', end='    '); 
        print('|   ', ' ' * 6, '|', ' ' * 6, '   |')
        
        print(color(self.state[13]), '-' * 9, color(self.state[14]), '-' * 9, color(self.state[15]), end='    '); 
        print('n', '-' * 9, 'o', '-' * 9, 'p')

    def check_put(self, pos):
        """
        This function checks whether the PUT-movement is a legal movement. 
        The input arguments are the putting position.
        The output is a boolean variable.
        """
        valid_flag = True
        
        ### TODO
        if pos < 0 or pos > 15 or self.state[pos] != '.':
            valid_flag = False
            
        return valid_flag

    def check_move(self, s, t, player):
        """
        This function checks whether the MOVE-movement is a legal movement for the player. 
        The input arguments are the source position, the target position, and the player. 
        The output is a boolean variable.
        """
        valid_flag = True
        if s < 0 or s > 15 or t < 0 or t > 15: 
            valid_flag = False
        ### TODO
        if self.state[s] != player.get_symbol() or self.state[t] != '.' or ([s,t] not in self.edges and [t,s] not in self.edges):
            valid_flag = False

        return valid_flag

    def check_remove(self, pos, player):
        """
        This function checks whether the REMOVE-movement is a legal movement for the player. 
        The input arguments are the position on which the piece is expected to be removed and the player. 
        The output is a boolean variable.
        """
        valid_flag = True
        if pos < 0 or pos > 15:
            valid_flag = False
        ### TODO
        if self.state[pos] != player.get_symbol() or self.state[pos] == '.':
            valid_flag = False
        
        return valid_flag

    def put_piece(self, pos, player):
        """This function does the PUT-movement on the board without checking. 
        """
        ### TODO
        self.state[pos] = player.get_symbol()

    def move_piece(self, s, t, player):
        """This function does the MOVE-movement on the board without checking.
        """
        ### TODO
        self.state[s], self.state[t] = '.', self.state[s]

    def remove_piece(self, pos, player):
        """This function does the REMOVE-movement on the board without checking.
        """
        self.state[pos] = '.'

    def form_mill(self, pos, player):
        """
        This function returns a Boolean variable that represents whether it forms a mill at this position for the player.
        """
        if_form = False
        
        ### TODO
        for mill in self.mills:
            if pos in mill:
                flag = True
                for other_pos in mill:
                    if self.state[other_pos] != self.state[pos]:
                        flag = False
                if flag:
                    if_form = True
                    break        

        return if_form

    def check_win(self, player, opponent):
        """
        This function returns a boolean variable that represents whether the current player wins. 
        If the current player wins the game, then it returns True. Otherwise, it returns False. 
        """
        if_win = False

        # Winning condition 1
        # check whether the opponent has less than 2 pieces. 
        num_pieces = 0

        ### TODO (check every position to calculate the number of pieces for the player)
        for symbol in self.state:
            if symbol == opponent.get_symbol():
                num_pieces += 1

        if num_pieces <= 2: 
            if_win = True
            #print("win condition 1")
        else:
            # Winning condition 2
            # check whether the opponent cannot move

            can_move = False
            for i in range(len(self.state)):
                if self.state[i] == opponent.get_symbol():
                    piece_can_move = False
                    for j, k in self.edges:
                        ### TODO (check every edge to check whether there is a legal move)
                        # if self.check_move(j, k, opponent) or self.check_move(k, j, opponent):
                        #     piece_can_move = True
                        #     print("can_move",i,j,k,)
                        #     break
                        if (j == i and self.state[k] == '.') or (k == i and self.state[j] == '.'):
                            piece_can_move = True
                            break
                    if piece_can_move:
                        can_move = True
                        break
            if not can_move: 
                if_win = True

        return if_win
