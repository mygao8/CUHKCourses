#! /usr/bin/perl
use warnings;
use strict;
require "./Bank.pm";
require "./Jail.pm";
require "./Land.pm";
require "./Player.pm";

our @game_board = (
    new Bank(), new Land(), new Land(), new Land(), new Land(), new Land(), new Land(), new Land(), new Land(), new Jail(),
    new Land(), new Land(), new Land(), new Land(), new Land(), new Land(), new Land(), new Land(),
    new Jail(), new Land(), new Land(), new Land(), new Land(), new Land(), new Land(), new Land(), new Land(), new Jail(),
    new Land(), new Land(), new Land(), new Land(), new Land(), new Land(), new Land(), new Land(),
);
our $game_board_size = @game_board;

our @players = (new Player("A"), new Player("B"));
our $num_players = @players;

our $cur_player_idx = 0;
our $cur_player = $players[$cur_player_idx];
our $cur_round = 0;
our $num_dices = 1;

srand(0); # don't touch

# game board printing utility. Used to show player position.
sub printCellPrefix {
    my $position = shift;
    my @occupying = ();
    foreach my $player (@players) {
        if ($player->{position} == $position && $player->{money} > 0) {
            push(@occupying, ($player->{name}));
        }
    }
    print(" " x ($num_players - scalar @occupying), @occupying);
    if (scalar @occupying) {
        print("|");
    } else {
        print(" ");
    }
}

sub printGameBoard {
    print("-"x (10 * ($num_players + 6)), "\n");
    for (my $i = 0; $i < 10; $i += 1) {
        printCellPrefix($i);
        $game_board[$i]->print();
    }
    print("\n\n");
    for (my $i = 0; $i < 8; $i += 1) {
        printCellPrefix($game_board_size - $i - 1);
        $game_board[-$i-1]->print();
        print(" "x (8 * ($num_players + 6)));
        printCellPrefix($i + 10);
        $game_board[$i+10]->print();
        print("\n\n");
    }
    for (my $i = 0; $i < 10; $i += 1) {
        printCellPrefix(27 - $i);
        $game_board[27-$i]->print();
    }
    print("\n");
    print("-"x (10 * ($num_players + 6)), "\n");
}

sub terminationCheck {
    # ...
    my $remain_players = 0;
    foreach my $player (@players){
        if ($player->{money} > 0){
            $remain_players += 1;
        }
    }

    if ($remain_players == 1){
        return 0;
    }
    return 1;
}

sub throwDice {
    my $step = 0;
    for (my $i = 0; $i < $num_dices; $i += 1) {
        $step += 1 + int(rand(6));
    }
    return $step;
}

sub getInput{
    my $prompt = shift;
    print($prompt."\n");
    my $usr_input = <STDIN>;
    while ($usr_input ne "y\n" and $usr_input ne "n\n"){
        print($prompt."\n");
        $usr_input = <STDIN>;
    }
    return $usr_input eq "y\n";
}

sub getNextPlayer{
    $cur_player_idx += 1;
    while ($players[$cur_player_idx % $num_players]->{money} <= 0){
        $cur_player_idx += 1;
    }
    return $players[$cur_player_idx % $num_players];
}

sub main {
    while (terminationCheck()){
        printGameBoard();
        foreach my $player (@players) {
            $player->printAsset();
        }

        # ...
        # fixed cost each round
        if ($cur_player->{money} < 200){
            local $Player::due = $cur_player->{money};
            local $Player::handling_fee_rate = 0;
            $cur_player->payDue();
        }
        else{
            $cur_player->payDue();
        }

        my $cur_name = $cur_player->{name};
        print("Player ".$cur_name."'s turn.\n");

        # choice to throw two dice
        if (getInput("Pay \$500 to throw two dice? [y/n]")){
            if ($cur_player->{money} < 500*(1+0.05)){
                print("You do not have enough money to throw two dice!\n");
                $num_dices = 1;
            }
            else{
                local $Player::due = 500;
                local $Player::handling_fee_rate = 0.05;
                $num_dices = 2;
                $cur_player->payDue();
            }
        }
        else{
            $num_dices = 1;
        }

        my $step = throwDice();
        print("Points of dice: $step\n");

        # update pos and print board
        $cur_player->move($step);
        printGameBoard();

        # call stepOn according to where cur_player is
        $game_board[$cur_player->{position}]->stepOn();

        if (!terminationCheck()){
            last;
        }

        $cur_player = getNextPlayer();
        
        # if next player (cur_player now) is in jail, change cur_player to the next one
        while ($cur_player->{num_rounds_in_jail} > 0){
            $cur_player->payDue();
            $cur_player->{num_rounds_in_jail} -= 1;
            if (!terminationCheck()){
                last;
            }
            $cur_player = getNextPlayer();
        }
    }

    my $winner = getNextPlayer()->{name};
    print("Game over! winner: $winner.\n");
}

main();
