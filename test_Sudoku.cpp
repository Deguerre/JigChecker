
#include "ExactCover.h"

#include <iostream>
#include <format>

#include <chrono>

void TestSudoku(ExactCover cov, const char** board, ExactCover::id_t moves[9][9][9])
{
	// Create additional constraints for the given values

	for (int i = 0; i < 9; ++i) {
		for (int j = 0; j < 9; ++j) {
			int k = board[i][j] - '0';
			if (!k) continue;
			cov.ConstrainMove(moves[i][j][k - 1], cov.AddConstraint(std::format("given_{}_{}_{}", i + 1, j + 1, k)));
		}
	}

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
		for (int i = 0; i < 9; ++i) {
			for (int j = 0; j < 9; ++j) {
				for (int k = 0; k < 9; ++k) {
					if (std::find(s.begin(), s.end(), moves[i][j][k]) != s.end()) {
						std::cout << (k + 1);
					}
				}

			}
			std::cout << '\n';
		}
		std::cout << '\n';
	}
}

int main()
{
	ExactCover cov;
	cov.SolutionSize(81);

	// Create the Sudoku constraints

	// rowConstraints[i][k] means that row i contains value k
	ExactCover::id_t rowConstraints[9][9];
	// colConstraints[j][k] means that column j contains value k
	ExactCover::id_t colConstraints[9][9];
	// boxConstraints[b][k] means that box b contains value k
	ExactCover::id_t boxConstraints[9][9];
	// occConstraints[i][j] means that row i column j contains a value
	ExactCover::id_t occConstraints[9][9];

	for (int i = 0; i < 9; ++i) {
		for (int j = 0; j < 9; ++j) {
			rowConstraints[i][j] = cov.AddConstraint(std::format("row_{}_{}", i + 1, j + 1));
			colConstraints[i][j] = cov.AddConstraint(std::format("col_{}_{}", i + 1, j + 1));
			boxConstraints[i][j] = cov.AddConstraint(std::format("box_{}_{}", i + 1, j + 1));
			occConstraints[i][j] = cov.AddConstraint(std::format("occ_{}_{}", i + 1, j + 1));
		}
	}

	// Create all possible moves

	// moves[i][j][k] means that value k was placed in row i column j
	ExactCover::id_t moves[9][9][9];
	for (int i = 0; i < 9; ++i) {
		for (int j = 0; j < 9; ++j) {
			int b = (i / 3) * 3 + (j / 3);
			for (int k = 0; k < 9; ++k) {
				auto move = cov.AddMove(std::format("{}_{}_{}", i + 1, j + 1, k + 1));
				moves[i][j][k] = move;

				cov.ConstrainMove(move, rowConstraints[i][k]);
				cov.ConstrainMove(move, colConstraints[j][k]);
				cov.ConstrainMove(move, boxConstraints[b][k]);
				cov.ConstrainMove(move, occConstraints[i][j]);
			}
		}
	}

	// Test sudoku puzzles

	// Platinum Blonde
	static const char* platinumBlonde[9] = {
		"000000012",
		"000000003",
		"002300400",
		"001800005",
		"060070800",
		"000009000",
		"008500000",
		"900040500",
		"470006000"
	};
	TestSudoku(cov, platinumBlonde, moves);

	// AI Escargot by Arto Inkala
	static const char* aiEscargot[9] = {
		"800000000",
		"003600000",
		"070090200",
		"050007000",
		"000045700",
		"000100030",
		"001000068",
		"008500010",
		"090000400"
	};
	TestSudoku(cov, aiEscargot, moves);

	return 0;
}
