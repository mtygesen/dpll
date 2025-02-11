#include <Dpll.hpp>

int main(void) {
    dpll::Formula formula({{"p", "q"}, {"!p", "r"}, {"!q", "!r"}});  
    dpll::Dpll solver;
    solver.solve(formula);
    return 0;
}