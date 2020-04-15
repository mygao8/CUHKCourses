import random

random.seed(0) # don't touch!

# you are not allowed to modify Player class!
class Player:
    due = 200
    income = 0
    tax_rate = 0.2
    handling_fee_rate = 0
    prison_rounds = 2

    def __init__(self, name):
        self.name = name
        self.money = 100000
        self.position = 0
        self.num_rounds_in_jail = 0

    def updateAsset(self):
        self.money += Player.income

    def payDue(self):
        self.money += Player.income * (1 - Player.tax_rate)
        self.money -= Player.due * (1 + Player.handling_fee_rate)

    def printAsset(self):
        print("Player %s's money: %d" % (self.name, self.money))

    def putToJail(self):
        self.num_rounds_in_jail = Player.prison_rounds

    def move(self, step):
        if self.num_rounds_in_jail > 0:
            self.num_rounds_in_jail -= 1
        else:
            self.position = (self.position + step) % 36



class Bank:
    def __init__(self):
        pass

    def print(self):
        print("Bank ", end='')

    def stepOn(self):
        # ...
        global cur_player
        setPay(cur_player, 0, 0)
        setRecv(cur_player, 2000, 0)
        cur_player.payDue()
        # ?? should recover?
        # Player.income = 0
        # Player.tax_rate = 0.2
        print("You received $2000 from the Bank!")
        return

class Jail:
    def __init__(self):
        pass

    def print(self):
        print("Jail ", end='')

    def stepOn(self):
        # ...                
        global cur_player
        if getInput("Pay $1000 to reduce the prison round to 1? [y/n]\n"):
            setRecv(cur_player, 0, 0)
            if setPay(cur_player, 1000, 0.1):
                Player.prison_rounds = 1
                cur_player.payDue()
            else:
                print("You do not have enough money to reduce the prison round!")
                Player.prison_rounds = 2
        else:
            Player.prison_rounds = 2

        cur_player.putToJail()


class Land:
    land_price = 1000
    upgrade_fee = [1000, 2000, 5000]
    toll = [500, 1000, 1500, 3000]
    tax_rate = [0.1, 0.15, 0.2, 0.25]

    def __init__(self):
        self.owner = None
        self.level = 0

    def print(self):
        if self.owner is None:
            print("Land ", end='')
        else:
            print("%s:Lv%d" % (self.owner.name, self.level), end="")
    
    def buyLand(self):
        # ...
        global cur_player
        
        setRecv(cur_player, 0, 0)
        if setPay(cur_player, Land.land_price, 0.1):
            self.owner = cur_player
            self.level = 0
        else:
            print("You do not have enough money to buy the land!")

        cur_player.payDue()
    
    def upgradeLand(self):
        # ...
        global cur_player
        
        setRecv(cur_player, 0, 0)
        if setPay(cur_player, Land.upgrade_fee[self.level], 0.1):
            self.level += 1
        else:
            print("You do not have enough money to upgrade the land!")

        cur_player.payDue()
    
    def chargeToll(self):
        # ...
        global cur_player

        setRecv(cur_player, 0, 0)
        if setPay(cur_player, Land.toll[self.level], 0):
            income = Land.toll[self.level]
        else:
            setPay(cur_player, cur_player.money, 0)     # pay the amount the player has
            income = cur_player.money

        cur_player.payDue()

        # ...
        setPay(self.owner, 0, 0)
        setRecv(self.owner, income, Land.tax_rate[self.level])
      
        self.owner.payDue()

    def stepOn(self):
        # ... 
        global cur_player
        # print("Land:\n", cur_player, self.owner)
        if self.owner == None:
            if getInput("Pay $1000 to buy the land? [y/n]\n"):
                self.buyLand()
        elif self.owner == cur_player:
            if self.level >= 3:
                return
            if getInput("Pay ${} to upgrade the land? [y/n]\n".format(Land.upgrade_fee[self.level])):
                self.upgradeLand()
        else:
            print("You need to pay player {} ${}".format(self.owner.name, Land.toll[self.level]))
            self.chargeToll()
        return

players = [Player("A"), Player("B")]
cur_player = players[0]
num_players = len(players)
cur_player_idx = 0
cur_player = players[cur_player_idx]
num_dices = 1
cur_round = 0

game_board = [
    Bank(), Land(), Land(), Land(), Land(), Land(), Land(), Land(), Land(), Jail(),
    Land(), Land(), Land(), Land(), Land(), Land(), Land(), Land(),
    Jail(), Land(), Land(), Land(), Land(), Land(), Land(), Land(), Land(), Jail(),
    Land(), Land(), Land(), Land(), Land(), Land(), Land(), Land()
]
game_board_size = len(game_board)

def setPay(player, due, fee_rate):
    if player.money >= due * (1+fee_rate):
        Player.handling_fee_rate = fee_rate
        Player.due = due
        return True
    else:
        Player.handling_fee_rate = 0
        Player.due = 0
        return False

def setRecv(player, income, tax_rate):
    Player.income = income
    Player.tax_rate = tax_rate

def isValid(usr_input):
    return usr_input == "y" or usr_input == "n"

def getInput(prompt):
    usr_input = input(prompt)
    while not isValid(usr_input):
        usr_input = input(prompt)
    return usr_input == "y"

def getNextPlayer():
    global cur_player_idx
    cur_player_idx += 1
    while players[cur_player_idx % num_players].money <= 0:
        cur_player_idx += 1
    return players[cur_player_idx % num_players]

def fixedCost(cur_player):
    # fixed cost each round
    setRecv(cur_player, 0, 0)
    if setPay(cur_player, 200, 0):
        pass
    else:
        setPay(cur_player, cur_player.money, 0)     # pay the amount the player has
    
    cur_player.payDue()

def printCellPrefix(position):
    occupying = []
    for player in players:
        if player.position == position and player.money > 0:
            occupying.append(player.name)
    print(" " * (num_players - len(occupying)) + "".join(occupying), end='')
    if len(occupying) > 0:
        print("|", end='')
    else:
        print(" ", end='')


def printGameBoard():
    print("-" * (10 * (num_players + 6)))
    for i in range(10):
        printCellPrefix(i)
        game_board[i].print()
    print("\n")
    for i in range(8):
        printCellPrefix(game_board_size - i - 1)
        game_board[-i - 1].print()
        print(" " * (8 * (num_players + 6)), end="")
        printCellPrefix(i + 10)
        game_board[i + 10].print()
        print("\n")
    for i in range(10):
        printCellPrefix(27 - i)
        game_board[27 - i].print()
    print("")
    print("-" * (10 * (num_players + 6)))


def terminationCheck():
    # ...
    # Return 1 if the game has not ended. Otherwise, return 0.
    remain_players = 0
    for player in players:
        if player.money > 0:
            remain_players += 1

    if remain_players == 1:
        return False
    return True


def throwDice():
    step = 0
    for i in range(num_dices):
        step += random.randint(1, 6)
    return step


def main():
    global cur_player
    global num_dices
    global cur_round
    global cur_player_idx

    while terminationCheck():
        printGameBoard()
        for player in players:
            player.printAsset()

        # fixed cost each round
        fixedCost(cur_player)

        print("Player {}'s turn.".format(cur_player.name))

        # choice to throw two dice
        if getInput("Pay $500 to throw two dice? [y/n]\n"):
            setRecv(cur_player, 0, 0)
            if setPay(cur_player, 500, 0.05):
                num_dices = 2
                cur_player.payDue()
            else:
                print("You do not have enough money to throw two dice!")
                num_dices = 1
        else:
            num_dices = 1


        step = throwDice()
        print("Points of dice: {}".format(step))

        # update pos and print board
        cur_player.move(step)
        printGameBoard()

        # call stepOn according to where cur_player is
        game_board[cur_player.position].stepOn()
        
        if not terminationCheck():
            break

        cur_player = getNextPlayer()
        # if next player (cur_player now) is in jail, change cur_player to the next one
        while cur_player.num_rounds_in_jail > 0:
            fixedCost(cur_player)
            cur_player.num_rounds_in_jail -= 1
            if not terminationCheck():
                break
            cur_player = getNextPlayer()
        
    print("Game over! winner: {}.".format(getNextPlayer().name))

    # ...


if __name__ == '__main__':
    main()
