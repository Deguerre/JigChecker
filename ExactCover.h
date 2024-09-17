#pragma once

#include <cstdint>
#include <deque>
#include <vector>
#include <string>

class ExactCover {
public:
	typedef std::uint32_t id_t;

	typedef std::vector<id_t> solution_t;

	ExactCover();
	~ExactCover();
	ExactCover(const ExactCover&);
	ExactCover& operator=(const ExactCover&);

	void SolutionSize(std::size_t size);

	id_t AddConstraint(const std::string& name, bool mandatory = true);

	const std::string& ConstraintName(id_t id);

	id_t AddMove(const std::string& name);
	const std::string& MoveName(id_t id);

	void ConstrainMove(id_t move, id_t constrant);

	std::uint32_t Solve(std::deque<solution_t>& spud);

private:
	struct Constraint;
	struct Move;
	struct Entry;

	id_t constraintHead, moveHead;
	std::vector<Move> moves;
	std::vector<Constraint> constraints;
	std::vector<Entry> entries;
	std::size_t solutionSize = 0;

	void Unlink(id_t id);
	void Link(id_t id);
	void LinkVert(id_t id);
	void UnlinkVert(id_t id);

	id_t MakeEntry();

	void RecursiveSolve(solution_t& curSolution, std::deque<solution_t>& spud);

	void CoverColumn(id_t chead);
	void UncoverColumn(id_t chead);
};
