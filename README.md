# JigChecker

This is my entry into the 2024 Improve On Terrible Python Code contest.

My solution to the jig checker problem is to recast it as an exact cover problem, then use Knuth's Algorithm X to solve it, specifically the [DLX realisation]( https://arxiv.org/abs/cs/0011047 ).

On my machine, this solves [Matt Parker's sample layout]( https://www.dropbox.com/scl/fi/dvylpftby1uzlrmpdt1mp/matt-jigsaw-layout.pdf?rlkey=512hqdr9y9h5mxjonhxiwhsy4&e=1&dl=0 ) in 94ms, correctly finding 8 solutions: two unique solutions, with four rotations of the whole grid.

## Code

The main solver code is in `ExactCover`. This may seem overly complex, using indices rather than pointers, but there is a reason for this. The whole solver is copyable, which means you could set up a problem in advance, and then solve instances of it on different threads with little difficulty.

This is illustrated (albeit not in a multi-threaded way) in `test_Sudoku`, which sets up the rules for Sudoku once, and then uses the solver to solve two different puzzles.

## Translating jigsaws to the exact cover problem

The exact cover constraints used in the translation are reasonably straightforward. As with standard tiling puzles, every piece must be used exactly once, and every cell must be occupied exactly once. There are additional constraints to ensure that the edges of the puzzle grid use flat edges.

To model the internal edges, we add additional "slots" at each horizontal and vertical internal edge, numbered from -p to p, where p is the number of tab types.

Let's say that we have 3 tab types, and a vertical edge with tab type 1. Then the piece to the left of that edge occupies slot 1, and the piece to the right of the edge occupies slots -3, -2, -1, 0, 2, and 3. This way, in a correctly-fit edge, all of the slots are occupied exactly once.

Note that tab type 0 is the flat edge. This is convenient, since the tab is also its own blank. This, conveniently, allows us to also solve puzzles with flat internal edges.
