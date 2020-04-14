use strict;
use warnings;
require "./Player.pm";

package Jail;
sub new {
    my $class = shift;
    my $self  = {};
    bless $self, $class;
    return $self;
}

sub print {
    print("Jail ");
}

sub stepOn {
    # ...
    my $self = shift;
    local $Player::prison_rounds = 2;
    if (main::getInput("Pay \$1000 to reduce the prison round to 1? [y/n]")){
        if ($main::cur_player->{money} < 1000 * (1+0.1)){
            print("You do not have enough money to reduce the prison round!\n");
        }
        else{
            local $Player::due = 1000;
            local $Player::handling_fee_rate = 0.1;
            $Player::prison_rounds = 1;

            $main::cur_player->payDue();
        }
    }

    $main::cur_player->putToJail();
}

1;
