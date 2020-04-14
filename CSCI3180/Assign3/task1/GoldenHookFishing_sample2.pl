use strict;
use warnings;

package GoldenHookFishing;
use Game;

my $game = Game->new();
if ($game->setPlayers([])) {
	$game->startGame();
}
