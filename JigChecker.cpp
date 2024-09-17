
#include "ExactCover.h"

#include <iostream>
#include <format>

#include <chrono>

int main()
{
	ExactCover cov;

	// Pieces are cribbed from:
	// https://www.dropbox.com/scl/fi/dvylpftby1uzlrmpdt1mp/matt-jigsaw-layout.pdf
	//
	// Edges of each piece are in clocwise order starting from the top.

	constexpr int NUM_TAB_TYPES = 7, EDGE_BIAS = NUM_TAB_TYPES;
	constexpr int SIDE_LENGTH = 5;
	constexpr int NUM_PIECES = SIDE_LENGTH * SIDE_LENGTH;
	static int pieces[NUM_PIECES][4] = {
		{ 0, 1, -1, 0 },
		{ 0, 1, -2, -1 },
		{ 0, 1, -5, -1 },
		{ 0, -3, 5, -1 },
		{ 0, 0, 6, 3 },
		{ 1, -6, -7, 0 },
		{ 2, 2, 5, 6 },
		{ 5, -5, -2, -2 },
		{ -5, 4, -3, 5 },
		{ -6, 0, -4, -4 },
		{ 7, -6, -7, 0 },
		{ -5, -5, -5, 6 },
		{ 2, -2, 4, 5 },
		{ 3, -3, -6, 2 },
		{ 4, 0, 1, 3 },
		{ 7, -4, 4, 0 },
		{ 5, -2, 3, 4 },
		{ -4, -3, -2, 2 },
		{ 6, -3, -4, 3 },
		{ -1, 0, 7, 3 },
		{ -4, -7, 0, 0 },
		{ -3, -7, 0, 7 },
		{ 2, -6, 0, 7 },
		{ 4, -1, 0, 6 },
		{ -7, 0, 0, 1 }
	};

	// useConstraint[i] means piece i is used
	ExactCover::id_t useConstraint[NUM_PIECES];
	// occConstraint[j] means place j is occupied
	ExactCover::id_t occConstraint[NUM_PIECES];

	for (int i = 0; i < NUM_PIECES; ++i) {
		useConstraint[i] = cov.AddConstraint(std::format("use_{}", i));
		occConstraint[i] = cov.AddConstraint(std::format("occ_{}_{}", i / SIDE_LENGTH, i % SIDE_LENGTH));
	}

	// vertConstraint[x][y][e+EDGE_BIAS] means that edge (x,y) is edge type e.
	ExactCover::id_t vertConstraint[SIDE_LENGTH + 1][SIDE_LENGTH][NUM_TAB_TYPES * 2 + 1];
	ExactCover::id_t horzConstraint[SIDE_LENGTH][SIDE_LENGTH +1][NUM_TAB_TYPES * 2 + 1];
	for (int i = 1; i < SIDE_LENGTH; ++i) {
		for (int j = 0; j < SIDE_LENGTH; ++j) {
			for (int k = -NUM_TAB_TYPES; k <= NUM_TAB_TYPES; ++k) {
				vertConstraint[i][j][k + EDGE_BIAS] = cov.AddConstraint(std::format("vert_{}_{}_{}", i, j, k));
				horzConstraint[j][i][k + EDGE_BIAS] = cov.AddConstraint(std::format("horz_{}_{}_{}", j, i, k));
			}
		}
	}

	// uEdgeConstraint[i] means that the upped edge of the whole grid at i is flat (i.e. edge zero).
	ExactCover::id_t uEdgeConstraint[SIDE_LENGTH];
	ExactCover::id_t dEdgeConstraint[SIDE_LENGTH];
	ExactCover::id_t lEdgeConstraint[SIDE_LENGTH];
	ExactCover::id_t rEdgeConstraint[SIDE_LENGTH];
	for (int i = 0; i < SIDE_LENGTH; ++i) {
		uEdgeConstraint[i] = cov.AddConstraint(std::format("u_{}", i));
		dEdgeConstraint[i] = cov.AddConstraint(std::format("d_{}", i));
		lEdgeConstraint[i] = cov.AddConstraint(std::format("l_{}", i));
		rEdgeConstraint[i] = cov.AddConstraint(std::format("r_{}", i));
	}

	ExactCover::id_t moves[NUM_PIECES][NUM_PIECES][4];
	for (int i = 0; i < NUM_PIECES; ++i) {
		for (int y = 0; y < SIDE_LENGTH; ++y) {
			for (int x = 0; x < SIDE_LENGTH; ++x) {
				int j = y * 5 + x;
				for (int o = 0; o < 4; ++o) {
					static int orientations[4][4] = {
						{ 0, 1, 2, 3 },
						{ 3, 0, 1, 2 },
						{ 2, 3, 0, 1 },
						{ 1, 2, 3, 0 }
					};

					ExactCover::id_t move = cov.AddMove(std::format("x{}_y{}_p{}_o{}", x, y, i, o));

					int u = -pieces[i][orientations[o][0]];
					int r = pieces[i][orientations[o][1]];
					int d = pieces[i][orientations[o][2]];
					int l = -pieces[i][orientations[o][3]];

					cov.ConstrainMove(move, useConstraint[i]);
					cov.ConstrainMove(move, occConstraint[j]);
					for (int k = -NUM_TAB_TYPES; k <= NUM_TAB_TYPES; ++k) {
						if (k != u && y > 0) {
							cov.ConstrainMove(move, horzConstraint[x][y][k + EDGE_BIAS]);
						}
						if (k == d && y + 1 < SIDE_LENGTH) {
							cov.ConstrainMove(move, horzConstraint[x][y + 1][k + EDGE_BIAS]);
						}
						if (k != l && x > 0) {
							cov.ConstrainMove(move, vertConstraint[x][y][k + EDGE_BIAS]);
						}
						if (k == r && x + 1 < SIDE_LENGTH) {
							cov.ConstrainMove(move, vertConstraint[x + 1][y][k + EDGE_BIAS]);
						}
					}
					if (x == 0 && !l) cov.ConstrainMove(move, lEdgeConstraint[y]);
					if (x == SIDE_LENGTH - 1 && !r) cov.ConstrainMove(move, rEdgeConstraint[y]);
					if (y == 0 && !u) cov.ConstrainMove(move, uEdgeConstraint[x]);
					if (y == SIDE_LENGTH - 1 && !d) cov.ConstrainMove(move, dEdgeConstraint[x]);
					moves[i][j][o] = move;
				}
			}
		}
	}

	cov.SolutionSize(NUM_PIECES);

	std::deque<ExactCover::solution_t> spud;
	{
		using namespace std::chrono;

		auto startTime = system_clock::now();
		cov.Solve(spud);
		auto endTime = system_clock::now();
		std::cout << "Solve time: " << duration_cast<microseconds>(endTime - startTime).count() / 1000.0 << "ms\n";
	}

	std::cout << "Number of solutions: " << spud.size() << '\n';

	for (auto& s : spud) {
		for (auto i : s) {
			std::cout << ' ' << cov.MoveName(i);
		}
		std::cout << '\n';
	}

	return 0;
}
