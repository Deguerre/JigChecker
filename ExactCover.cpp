#include "ExactCover.h"
#include "ExactCover.h"
#include "ExactCover.h"
#include <limits>
#include <iostream>

#undef VERBOSE_DEBUG

template<typename Container>
void EnsureCapacity(Container& container, std::size_t required = 1)
{
    uint64_t oldSize = container.size();
    uint64_t oldCapacity = container.capacity();
    uint64_t newCapacity = oldSize + required;
    if (newCapacity < oldCapacity)
    {
        newCapacity = std::min<std::size_t>(newCapacity,
            (oldCapacity * 3 + 1) / 2);
        newCapacity = std::max<std::size_t>(newCapacity, 4);
        container.reserve(newCapacity);
    }
}


struct ExactCover::Entry {
    id_t left, right, up, down;
    id_t constraint = std::numeric_limits<id_t>::max();
    id_t move = std::numeric_limits<id_t>::max();

    Entry(id_t id)
        : left(id), right(id), up(id), down(id)
    {
    }
};


struct ExactCover::Move {
    std::string name;
    id_t head = 0;

    Move(std::string name)
        : name(std::move(name))
    {
    }
};

struct ExactCover::Constraint {
    std::string name;
    id_t head = 0;
    std::size_t count = 0;

    Constraint(std::string name)
        : name(std::move(name))
    {
    }
};

ExactCover::ExactCover()
{
    constraintHead = constraints.size();
    constraints.emplace_back("MandatoryConstraintHead");
    auto& chead = constraints[constraintHead];
    constraints[constraintHead].head = MakeEntry();
    entries[constraints[constraintHead].head].constraint = constraintHead;

    moveHead = moves.size();
    moves.emplace_back("MoveHead");
    moves[moveHead].head = MakeEntry();
    entries[moves[moveHead].head].move = moveHead;
}

ExactCover::~ExactCover()
{
}

ExactCover::ExactCover(const ExactCover&) = default;

ExactCover& ExactCover::operator=(const ExactCover&) = default;

void ExactCover::SolutionSize(std::size_t size)
{
    solutionSize = size;
}

ExactCover::id_t ExactCover::MakeEntry()
{
    EnsureCapacity(entries);
    id_t id = entries.size();
    entries.emplace_back(id);
    return id;
}

ExactCover::id_t ExactCover::AddConstraint(const std::string& name, bool mandatory)
{
    EnsureCapacity(constraints);
    id_t id = constraints.size();
    constraints.emplace_back(name);
    auto entryid = MakeEntry();
    constraints[id].head = entryid;
    entries[entryid].constraint = id;

    if (mandatory) {
        auto& entry = entries[entryid];
        entry.left = entries[constraints[constraintHead].head].left;
        entry.right = constraints[constraintHead].head;
        Link(entryid);
    }
    return id;
}

const std::string& ExactCover::ConstraintName(id_t id)
{
    return constraints[id].name;
}

ExactCover::id_t ExactCover::AddMove(const std::string& name)
{
    EnsureCapacity(moves);
    id_t id = moves.size();
    moves.emplace_back(name);
    auto entryid = MakeEntry();
    moves[id].head = entryid;
    auto& mhead = entries[moveHead];
    auto& mentry = entries[entryid];
    mentry.move = id;
    mentry.up = mhead.up;
    mentry.down = moveHead;
    LinkVert(entryid);
    return id;
}

const std::string& ExactCover::MoveName(id_t id)
{
    return moves[id].name;
}

void ExactCover::ConstrainMove(id_t move, id_t constraint)
{
    auto id = MakeEntry();
    auto& entry = entries[id];

    entry.constraint = constraint;
    entry.move = move;
    ++constraints[constraint].count;

    auto cheadid = constraints[constraint].head;
    entry.up = entries[cheadid].up;
    entry.down = cheadid;

    auto mheadid = moves[move].head;
    entry.left = entries[mheadid].left;
    entry.right = mheadid;

    Link(id);
    LinkVert(id);
}


std::uint32_t ExactCover::Solve(std::deque<solution_t>& spud)
{
    solution_t curSolution;
    curSolution.reserve(solutionSize);
    RecursiveSolve(curSolution, spud);
    return spud.size();
}


void
ExactCover::CoverColumn(id_t constraint)
{
    auto& col = constraints[constraint];
    auto& centry = entries[col.head];

    Unlink(col.head);

    for (auto i = centry.down; i != col.head; i = entries[i].down) {
        auto rheadid = i;
        auto& rentry = entries[rheadid];
        for (auto j = rentry.right; j != rheadid; j = entries[j].right) {
            if (entries[j].constraint != std::numeric_limits<id_t>::max()) {
                UnlinkVert(j);
                --constraints[entries[j].constraint].count;
            }
        }
    }
}


void
ExactCover::UncoverColumn(id_t constraint)
{
    auto& col = constraints[constraint];
    auto& centry = entries[col.head];

    for (auto i = centry.up; i != col.head; i = entries[i].up) {
        // auto rheadid = moves[entries[i].move].head;
        auto rheadid = i;
        auto& rentry = entries[rheadid];
        for (auto j = rentry.left; j != rheadid; j = entries[j].left) {
            if (entries[j].constraint != std::numeric_limits<id_t>::max()) {
                LinkVert(j);
                ++constraints[entries[j].constraint].count;
            }
        }
    }

    Link(col.head);
}


void
ExactCover::RecursiveSolve(solution_t& curSolution, std::deque<solution_t>& spud)
{
#ifdef VERBOSE_DEBUG
    auto depth = curSolution.size();
#endif

    // Choose a constraint.

    id_t c = constraintHead;
    auto chead = constraints[c].head;
    std::size_t minSize = std::numeric_limits<size_t>::max();

    for (id_t i = entries[chead].right; i != chead; i = entries[i].right) {
        if (constraints[entries[i].constraint].count < minSize) {
            c = entries[i].constraint;
            minSize = constraints[c].count;
        }
    }

    if (!minSize) {
        // Unsatisfiable constraint.
#ifdef VERBOSE_DEBUG
        std::cerr << depth << ": Unsatisfiable constraint " << constraints[c].name  << "\n";
        for (auto m : curSolution) {
            std::cerr << ' ' << moves[m].name;
        }
        std::cerr << '\n';
#endif
        return;
    }

    // If all constraints are satisfied, we have a solution.
    if (c == constraintHead) {
#ifdef VERBOSE_DEBUG
        std::cerr << depth << ": Found solution\n";
#endif
        spud.push_back(curSolution);
        return;
    }

    // Otherwise, try to satisfy this constraint.
#ifdef VERBOSE_DEBUG
    std::cerr << depth << ": Using constraint " << constraints[c].name << " (size " << minSize << ")\n";
#endif
    CoverColumn(c);

    chead = constraints[c].head;
    for (id_t i = entries[chead].down; i != chead; i = entries[i].down) {
        auto move = entries[i].move;
#ifdef VERBOSE_DEBUG
        std::cerr << depth << ": Trying move " << moves[move].name << '\n';
#endif
        curSolution.emplace_back(move);

        for (id_t j = entries[i].right; j != i; j = entries[j].right) {
            if (entries[j].constraint != std::numeric_limits<id_t>::max()) {
                // std::cerr << depth << ": Eliminating constraint " << constraints[entries[j].constraint].name << '\n';
                CoverColumn(entries[j].constraint);
            }
        }

        RecursiveSolve(curSolution, spud);

#ifdef VERBOSE_DEBUG
        std::cerr << depth << ": Unwinding move " << moves[move].name << '\n';
#endif

        for (id_t j = entries[i].left; j != i; j = entries[j].left) {
            if (entries[j].constraint != std::numeric_limits<id_t>::max()) {
                UncoverColumn(entries[j].constraint);
            }
        }
        curSolution.pop_back();
    }

    UncoverColumn(c);
}


void ExactCover::Unlink(id_t id)
{
    auto& entry = entries[id];
    entries[entry.right].left = entry.left;
    entries[entry.left].right = entry.right;
}


void ExactCover::Link(id_t id)
{
    auto& entry = entries[id];
    entries[entry.right].left = id;
    entries[entry.left].right = id;
}


void ExactCover::LinkVert(id_t id)
{
    auto& entry = entries[id];
    entries[entry.down].up = id;
    entries[entry.up].down = id;
}


void ExactCover::UnlinkVert(id_t id)
{
    auto& entry = entries[id];
    entries[entry.down].up = entry.up;
    entries[entry.up].down = entry.down;
}
