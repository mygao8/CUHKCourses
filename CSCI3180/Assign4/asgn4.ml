(* 
CSCI3180 Principles of Programming Languages
--- Declaration ---
I declare that the assignment here submitted is original except for source
material explicitly acknowledged. I also acknowledge that I am aware of
University policy and regulations on honesty in academic work, and of the
disciplinary guidelines and procedures applicable to breaches of such policy
and regulations, as contained in the website
http://www.cuhk.edu.hk/policy/academichonesty/
Assignment 4
Name : GAO Ming Yuan
Student ID : 1155107738
Email Addr : 1155107738@link.cuhk.edu.hk
*)

(* 
test case:
[(Clubs, 13), (Clubs, 11), (Spades, 7), (Spades, 3), (Hearts, 2)]
[(Spades, 11), (Spades, 9), (Hearts, 8), (Diamonds, 8), (Diamonds, 3)]
[(Clubs, 13), (Spades, 13), (Hearts, 6), (Spades, 1), (Diamonds, 1)]
[(Clubs, 10), (Clubs, 9), (Hearts, 9), (Spades, 9), (Spades, 3)]
[(Diamonds, 6), (Clubs, 6), (Spades, 6), (Spades, 4), (Diamonds, 4)]
[(Diamonds, 11), (Spades, 11), (Clubs, 11), (Hearts, 11), (Hearts, 10)] 

[(Clubs, 13), (Clubs, 11), (Clubs, 7), (Clubs, 3), (Clubs, 2)]
[(Spades, 10), (Spades, 9), (Spades, 8), (Spades, 7), (Spades, 1)]
[(Clubs, 13), (Clubs, 12), (Spades, 11), (Spades, 10), (Hearts, 1)]
[(Clubs, 5), (Clubs, 4), (Spades, 3), (Spades, 2), (Hearts, 1)]
*)

datatype suit = Clubs | Diamonds | Hearts | Spades;
datatype hand = Nothing | Pair | Two_Pairs | Three_Of_A_Kind |
Full_House | Four_Of_A_Kind | Flush | Straight;

fun gt_suit (x:suit, _):suit = x;
fun gt_rank (_, x:int):int = x;

(* 3(1)  takes a list of five cards and returns if the hand is a flush.*)
fun check_flush (L: (suit * int)list):bool = 
    if length(L) = 1 then true
    else 
        if gt_suit(hd(L)) = gt_suit(hd(tl(L))) andalso check_flush (tl(L)) then true 
        else false;

(* 3(2)  takes two flush card lists. The return value is a string 
selected from three candidates. i.e., "Hand 1 wins", "Hand 2 wins" and "This is a tie". *)
fun compare_flush ([], []) = "This is a tie"
|   compare_flush (Hand1: (suit * int)list, Hand2: (suit * int)list):string = 
    (* if check_straight(Hand1) andalso check_straight(Hand2) then compare_straight(Hand1, Hand2)
    else if check_straight(Hand1) then "Hand 1 wins"
    else if check_straight(Hand2) then "Hand 2 wins"
    else *)
        if gt_rank(hd(Hand1)) = gt_rank(hd(Hand2)) then compare_flush(tl(Hand1), tl(Hand2))
        else 
            if gt_rank(hd(Hand1)) > gt_rank(hd(Hand2)) then "Hand 1 wins"
            else "Hand 2 wins";

(* 3(3)  takes a list of five cards and returns if the hand is a straight. *)
fun check_straight (L: (suit * int)list):bool = 
    if length(L) = 1 then true 
    else 
        if (gt_rank(hd(L)) = gt_rank(hd(tl(L)))+1 orelse (gt_rank(hd(L)) = 10 andalso gt_rank(hd(tl(L))) = 1))
                andalso check_straight (tl(L)) then true
        else false;

(* 3(4)  takes two straight card lists. The return value is a string 
selected from three candidates. i.e., "Hand 1 wins", "Hand 2 wins" and "This is a tie". *)
fun compare_straight ([], []) = "This is a tie"
|   compare_straight ([(_, 1)], [(_, 9)]) = "Hand 1 wins"
|   compare_straight ([(_, 9)], [(_, 1)]) = "Hand 2 wins"
|   compare_straight (Hand1: (suit * int)list, Hand2: (suit * int)list):string = 
    if gt_rank(hd(Hand1)) = gt_rank(hd(Hand2)) then compare_straight(tl(Hand1), tl(Hand2))
    else
        if gt_rank(hd(Hand1)) > gt_rank(hd(Hand2)) then "Hand 1 wins"
        else "Hand 2 wins";

(* 3(5) takes a list of five cards and returns the hand type (Nothing, Pair, Two Pairs, 
Three of a Kind, Full House, Four of a Kind) and a list of rank-quantity pairs.  
Note that the list should be ordered according to the order of comparison of each type of hand. *)
fun first (x:int, _):int = x;
fun second (_, x:int):int = x;

fun insert(Rank, Number, [], NewL) = NewL@[(Rank, Number)]
|   insert(Rank:int, Number:int, OldL:(int*int)list, NewL:(int*int)list):(int*int)list =
    if Number > second(hd(OldL)) orelse (Number = second(hd(OldL)) andalso Rank > first(hd(OldL))) then NewL@(Rank, Number)::OldL
    else insert(Rank, Number, tl(OldL), NewL@[hd(OldL)])

fun map(Rank, Number, [], NewL) = insert(Rank, Number, NewL, [])
|   map(Rank:int, Number:int, L:(suit*int)list, NewL:(int*int)list):(int*int)list = 
        if gt_rank(hd(L)) = Rank then map(Rank, Number+1, tl(L), NewL)
        else map(gt_rank(hd(L)), 0, L, insert(Rank, Number, NewL, []))

fun count_patterns (L: (suit * int)list):(hand * (int*int)list) = 
    let 
        val Res = map(gt_rank(hd(L)), 0, L, [])
        val HandType = (second(hd(Res)), second(hd(tl(Res))))
        fun gt_hand(4, 1) = (Four_Of_A_Kind, Res)
        |   gt_hand(3, 2) = (Full_House, Res)
        |   gt_hand(3, 1) = (Three_Of_A_Kind, Res)
        |   gt_hand(2, 2) = (Two_Pairs, Res)
        |   gt_hand(2, 1) = (Pair, Res)
        |   gt_hand(1, 1) = (Nothing, Res);
    in
        gt_hand(HandType:(int*int)):(hand * (int*int)list)
    end

(* 3(6)  takes two card lists and returns a string 
selected from three candidates. i.e., "Hand 1 wins", "Hand 2 wins" and "This is a tie". *)
fun compare_rank([], []) = "This is a tie"
|   compare_rank( (Rank1, _)::Map1 :(int*int)list, (Rank2, _)::Map2 :(int*int)list):string = 
    if Rank1 = Rank2 then compare_rank (Map1, Map2)
    else if Rank1 > Rank2 then "Hand 1 wins"
    else "Hand 2 wins"

fun compare_count(Hand1: (suit * int)list, Hand2: (suit * int)list):string = 
    let
        val Map1 = map(gt_rank(hd(Hand1)), 0, Hand1, [])
        val Map2 = map(gt_rank(hd(Hand2)), 0, Hand2, [])
    in 
        if second(hd(Map1)) = second(hd(Map2)) andalso second(hd(tl(Map1))) = second(hd(tl(Map2))) then compare_rank(Map1, Map2)
        else if second(hd(Map1)) > second(hd(Map2)) orelse 
            (second(hd(Map1)) = second(hd(Map2)) andalso second(hd(tl(Map1))) > second(hd(tl(Map2)))) then "Hand 1 wins"
        else "Hand 2 wins"
    end