# Happy Bookcase

In a quiet part of our building, there are some rather strange bookcases. They are (like most bookcases) generally happy, but they become unhappy when their books are not arranged but using a dynamic (linked-list), so that you never have to make any assumptions about the maximum numbers of boards stored. After years of dedicated research, a team of scientists led by Simon Lock and Sion Hannuna came to understand the trick to making the bookcases happy again. It turned out that a bookcase is only happy if :

• Each shelf only has books of one colour (or is empty).
• All books of the same colour are on the same shelf.
• The only books that exist are black(K),red(R),green(G),yellow(Y),blue(B),magenta(M),
cyan(C) or white(W).

However, to make things worse, there are some complex rules about how books may be re- arranged :
1. You can only move one book at a time.
2. The only book that can move is the rightmost one from each shelf.
3. The book must move to become the rightmost book on its new shelf.
4. You can’t put more books on a shelf than its maximum size.
