#include <Dpll.hpp>

int main(void) {
    // (p ∨ q) ∧ (¬p ∨ r) ∧ (¬q ∨ ¬r)
    dpll::Formula formula({{"p", "q"}, {"!p", "r"}, {"!q", "!r"}});  
    dpll::Dpll solver;
    solver.solve(formula);
}