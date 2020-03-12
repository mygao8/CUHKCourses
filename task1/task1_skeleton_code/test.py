def print_board():
    def color(x):
        if x == '.': return x
        def g(s): return '\033[92m' + s + '\033[0m'
        def b(s): return '\033[94m' + s + '\033[0m'
        return g(x) if x == '#' else b(x)

    print(color(state[0]), '-' * 9, color(state[1]), '-' * 9, color(state[2]), end='    '); 
    print('a', '-' * 9, 'b', '-' * 9, 'c')
    print('|   ', ' ' * 6, '|', ' ' * 6, '   |', end='    '); 
    print('|   ', ' ' * 6, '|', ' ' * 6, '   |')
    print('|   ', color(state[3]), '-' * 4, color(state[4]), '-' * 4, color(state[5]), '   |', end='    '); 
    print('|   ', 'd', '-' * 4, 'e', '-' * 4, 'f', '   |')
    print('|   ' * 2, ' ' * 7, '   |' * 2, end='    '); 
    print('|   ' * 2, ' ' * 7, '   |' * 2)
    print(color(state[6]), '-', color(state[7]), ' ' * 13, color(state[8]), '-', color(state[9]), end='    '); 
    print('g', '-', 'h', ' ' * 13, 'i', '-', 'j', )
    print('|   ' * 2, ' ' * 7, '   |' * 2, end='    '); 
    print('|   ' * 2, ' ' * 7, '   |' * 2)
    print('|   ', color(state[10]), '-' * 4, color(state[11]), '-' * 4, color(state[12]), '   |', end='    '); 
    print('|   ', 'k', '-' * 4, 'l', '-' * 4, 'm', '   |')
    print('|   ', ' ' * 6, '|', ' ' * 6, '   |', end='    '); 
    print('|   ', ' ' * 6, '|', ' ' * 6, '   |')
    print(color(state[13]), '-' * 9, color(state[14]), '-' * 9, color(state[15]), end='    '); 
    print('n', '-' * 9, 'o', '-' * 9, 'p')

state = ['.' for i in range(16)]
i=0
while i < 100:
    print_board()