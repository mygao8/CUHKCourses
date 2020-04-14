use strict;
use warnings;
 
package Player;
sub new {
    my $class = shift @_;
    my @cards = (); # empty before passing in (from Deck.pm)
    my $self = {
        "name" => shift @_,
        "cards" =>  \@cards,
    };
    return bless $self, $class;
}

sub getCards {
    my $self = shift @_;
    my $cards = $self->{"cards"};
    my $taken = shift @_; # $taken is ref to array of the taken cards

    for (my $i = 0; $i <= $#$taken; $i = $i + 1) {
        push (@$cards, @$taken[$i]);
    };
}

sub dealCards {
    my $self = shift @_;
    my $cards = $self->{"cards"};  
    my $top = shift @$cards;

    # deal $top to game
    return $top;
}

sub numCards {
    my $self = shift @_;
    my $cards = $self->{"cards"};  
    my $size = @$cards;
    return $size;
}

return 1;