# Davis–Putnam–Logemann–Loveland (DPLL) algorithm
DPLL single-header include in C++20

## Example Usage
Formula must be encoded in CNF

E.g. to solve the following SAT problem: (p ∨ q) ∧ (¬p ∨ r) ∧ (¬q ∨ ¬r)
```cpp
#include <Dpll.hpp>
int main(void) {
    dpll::Formula formula({{"p", "q"}, {"!p", "r"}, {"!q", "!r"}});  
    dpll::Dpll solver;
    solver.solve(formula);
}
```

Which gives the following output:
```bash
Starting solver...
Formula is satisfiable!
Assignment:
r = false
q = false
p = true
```