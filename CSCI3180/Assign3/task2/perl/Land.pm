use strict;
use warnings;
package Land;

our @upgrade_fee = (1000, 2000, 5000);
our @toll = (500, 1000, 1500, 3000);
our @tax_rate = (0.1, 0.15, 0.2, 0.25);

sub new {
    my $class = shift;
    my $self  = {
        owner => undef,
        level => 0,
    };
    bless $self, $class;
    return $self;
}

sub print {
    my $self = shift;
    if (!defined($self->{owner})) {
        print("Land ");
    } else {
        print("$self->{owner}->{name}:Lv$self->{level}");
    }
}

sub buyLand {
    # ...
    my $self = shift;

    if ($main::cur_player->{money} < 1000 * (1+0.1)){
        print("You do not have enough money to buy the land!\n");
        return;
    }

    local $Player::due = 1000;
    local $Player::handling_fee_rate = 0.1;
    $self->{owner} = $main::cur_player;
    $self->{level} = 0;

    $main::cur_player->payDue();
}

sub upgradeLand {
    # ... 
    my $self = shift;

    if($main::cur_player->{money} < $upgrade_fee[$self->{level}] * (1+0.1)){
        print ("You do not have enough money to upgrade the land!\n");
        return;
    }

    local $Player::due = $upgrade_fee[$self->{level}];
    local $Player::handling_fee_rate = 0.1;
    $self->{level} += 1;

    $main::cur_player->payDue();
}

sub chargeToll {
    # ...
    my $self = shift;
    my $income;

    if ($main::cur_player->{money} >= $toll[$self->{level}]){
        # enough money to pay toll
        $income = $toll[$self->{level}];
    }
    else{
        $income = $main::cur_player->{money};
    }

    local $Player::due = $income;
    local $Player::handling_fee_rate = 0;
    
    $main::cur_player->payDue();

    # ...
    local $Player::due = 0;
    local $Player::income = $income;
    local $Player::tax_rate = $tax_rate[$self->{level}];
    
    $self->{owner}->payDue();
}

sub stepOn {
    # ...
    my $self = shift;
    if (!defined($self->{owner})){
        if (main::getInput("Pay \$1000 to buy the land? [y/n]")){
            $self->buyLand();
        }
    }
    elsif($self->{owner} == $main::cur_player){
        if ($self->{level} >= 3){
            return;
        }
        my $fee = $upgrade_fee[$self->{level}];
        if (main::getInput("Pay \$$fee to upgrade the land? [y/n]")){
            $self->upgradeLand();
        }
    }
    else{
        my $name = $self->{owner}->{name};
        my $fee = $toll[$self->{level}];
        print("You need to pay player $name \$$fee\n");
        $self->chargeToll();
    }
}
1;