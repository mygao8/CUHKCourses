% CSCI3180 Principles of Programming Languages
% --- Declaration ---
% I declare that the assignment here submitted is original except for source
% material explicitly acknowledged. I also acknowledge that I am aware of
% University policy and regulations on honesty in academic work, and of the
% disciplinary guidelines and procedures applicable to breaches of such policy
% and regulations, as contained in the website
% http://www.cuhk.edu.hk/policy/academichonesty/
% Assignment 4
% Name : GAO Ming Yuan
% Student ID : 1155107738
% Email Addr : 1155107738@link.cuhk.edu.hk


 % append/3
 append([], L, L).
 append([X|L1], L2, [X|L3]) :- append(L1, L2, L3).

% sum/3
sum(0, X, X).
sum(s(X), Y, s(Z)) :- sum(X, Y, Z).


% 1(a) Define element_last(X, L) which is true if the last element in list L is X.
element_last(X, L) :- append(_, [X], L).

% 1(b) Define element_n(X, L, N) which is true if the N-th element in list L is X.
element_n(X, L, s(0)) :- append([X], _, L).
element_n(X, [H|L], s(N)) :- element_n(X, L, N).

% 1(c) Define remove_n(X, L1, N, L2) which is true if the resulting list L2 is obtained from
%      L1 by removing the N-th element, and X is the removed element.
remove_n(H, [H|L1], s(0), L1).
remove_n(X, [H|L1], s(N), [H|L2]) :- remove_n(X, L1, N, L2).

% 1(d) Give a query to find which list will become [c,b,d,e] after 
%      removing its second element “a”.
% remove_n(a, L1, s(s(0)), [b,c,d,e]).

% 1(e) Define insert_n(X, L1, N, L2) which is true if the resulting list L2 is obtained by
%      inserting X to the position before the N-th element of list L1.
insert_n(X, L1, N, L2) :- remove_n(X, L2, N, L1).

% 1(f) Define repeat_three(L1, L2) which is true if the resulting list L2 has each element in
%      list L1 repeated three times.
repeat_three([], []).
repeat_three([H|L1], L2) :- 
    append([H,H,H], L2_T, L2),
    repeat_three(L1, L2_T).

% 1(g) Give a query to find which list will become [i,i,i,m,m,m,n,n,n] after 
%      repeating each element of it for three times.
% repeat_three(X, [i,i,i,m,m,m,n,n,n]).

% 2(a) Represent the multi-way tree in Figure 1 as a Prolog term, with order of the sub-trees
%      from left to right.
%mt(a, [mt(b, [mt(e, []), mt(f, [])]), mt(c, []), mt(d, [mt(g, [])])]).

% 2(b) Define the predicate is_tree(Term) which is true if Term represents a multi-way tree.
is_tree(mt(Root, [])).
is_tree(mt(Root, [Subtree|Forest])) :- 
    is_tree(Subtree), 
    is_tree(mt(Root, Forest)).

% 2(c) Define the predicate num_node(Tree, N) which is true if N is the number of nodes of
%      the given multi-way tree Tree.
num_node(mt(Root, []), s(0)).
num_node(mt(Root, [Subtree|Forest]), N) :- 
    num_node(Subtree, X),
    num_node(mt(Root, Forest), Y),
    sum(X, Y, N).

% 2(d)  Define sum_length(Tree, L) which is true if L is the sum of lengths of all internal
%       paths in Tree. 
sum_length(mt(Root, []), 0).
sum_length(mt(Root, [Subtree|Forest]), L) :-
    sum_length(Subtree, L1),
    num_node(Subtree, N1),
    sum(L1, N1, NewL1),
    sum_length(mt(Root, Forest), L2),
    sum(NewL1, L2, L).

len([], 0).
len([H|T], s(N)) :- len(T, N).
