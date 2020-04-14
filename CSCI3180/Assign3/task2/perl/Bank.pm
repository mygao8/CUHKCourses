use strict;
use warnings;
require "./Player.pm";

package Bank;
sub new {
    my $class = shift;
    my $self  = {};
    bless $self, $class;
    return $self;
}

sub print {
    print("Bank ");
}

sub stepOn {
    # ...
    my $self = shift;
    
    local $Player::due = 0;
    local $Player::income = 2000;
    local $Player::tax_rate = 0;

    $main::cur_player->payDue();
    print("You received \$2000 from the Bank!\n");
}

1;