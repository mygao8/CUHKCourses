use strict;
use warnings;

package Game;
use MannerDeckStudent; 
use Player;

sub new {
    my $class = shift @_;
    my $deck = MannerDeckStudent->new();
    my @players = ();   # array of obj Player
    my @cards = (); # @desktop_cards: array recording the desktop card stack
    my $self = {
        "deck" => $deck,
        "players" => \@players,
        "cards" => \@cards,
    };
    return bless $self, $class;
}

sub setPlayers {
    my ($self, $player_names) = @_;
    my $num_players = @$player_names;
    if ($num_players == 0 or 52 % $num_players != 0){
        print("Error: cards' number 52 can not be divided by players number $num_players!\n");
        return 0;
    }

    my $ref_players = $self->{"players"};
    for my $player_name (@$player_names){
        my $obj_player = Player->new($player_name);
        push (@$ref_players, $obj_player);
    };
    return 1;
}

sub getReturn {
    my $self = shift @_;
    my $cards = $self->{"cards"};
    my $target = @$cards[-1];
    my @ret = ();

    if ($target eq "J"){
        if ($#$cards != 0){
            # J is not the only one card on the desktop
            for (my $j = $#$cards; $j >= 0; $j = $j-1){
                push(@ret, @$cards[$j]);
            };        
            @$cards = ();   
        }
    }
    else{
        for (my $i = $#$cards - 1; $i >= 0; $i = $i - 1) {
            if (@$cards[$i] eq $target){
                for (my $j = $#$cards; $j >= $i; $j = $j-1){
                    my $tmp = pop(@$cards);
                    push(@ret, $tmp);
                };
                last;
            }
        };
    }

    return \@ret;
}

sub showCards {
	my $self = shift @_;
    my $cards = $self->{"cards"};

    for (my $i = 0; $i < $#$cards; $i = $i +1){
        print @$cards[$i]." ";
    };
    if ($#$cards >= 0){
        # @cards is not empty
        print @$cards[$#$cards];
    }
    print("\n");
}

sub startGame {
    my $self = shift @_;
    my $deck = $self->{"deck"};
    $deck->shuffle();

    # each player gets a deck of cards
    my $players = $self->{"players"}; # array of obj Player
    my $num_players = @$players;       
    my @ave_cards = $deck->AveDealCards($num_players);  # array of reference to array of ave cards
    for my $i (0..$#$players){
        @$players[$i]->{"cards"} = $ave_cards[$i];
    };

    print("There $num_players players in the game:\n");
    for my $i (1..$num_players){
        print @$players[$i-1]->{"name"}." ";
    };
    print("\n\nGame begin!!!\n");

    my $round = 0;
    while ($num_players>1){
        $round = $round + 1;
        my $idx_players = -1;
        for my $player (@$players){
            $idx_players = $idx_players + 1;
            if ($player == -1){
                redo;
            }

            print("\n");    # an empty line before each turn

            my $player_name = $player->{"name"};
            my $num_player_cards = $player->numCards();
            print("Player $player_name has $num_player_cards cards before deal.\n");
            
            print("=====Before player's deal=======\n");  
            $self->showCards();
            
            print("================================\n");
            my $player_deal = $player->dealCards();
            print("$player_name ==> card $player_deal\n");
            
            print("=====After player's deal=======\n");
            push(@{$self->{"cards"}}, $player_deal);
            my $player_taken = $self->getReturn(); # ref to array of cards taken by player
            $self->showCards();

            print("================================\n");
            $player->getCards($player_taken);
            $num_player_cards = $player->numCards();
            print("Player $player_name has $num_player_cards cards after deal.\n");
            if ($num_player_cards == 0){
                print("Player $player_name has no cards, out!\n");
                $num_players = $num_players-1;
                @$players[$idx_players] = -1;

                if ($num_players<=1){
                    last;
                }
            }
        };
    };

    if ($num_players == 1){
        # empty line for entering the first round, though no one deals
        print("\n");
    }
    for my $player (@$players){
        if ($player != -1){
            my $player_name = $player->{"name"};
            print("\nWinner is $player_name in game $round\n"); 
            last;
        }
    };
}

return 1;
