use strict;
use warnings;

package GoldenHookFishing;
use Game;

my $game = Game->new();
if ($game->setPlayers(['lin', 'liz'])) {
	$game->startGame();
}
