# Davis–Putnam–Logemann–Loveland (DPLL) algorithm
DPLL implementation as a single-header include in C++20

## Example Usage
All formulas must be encoded in conjunctive normal form (CNF)

E.g. to solve (p ∨ q) ∧ (¬p ∨ r) ∧ (¬q ∨ ¬r) ∧ (p ∨ ¬s) ∧ (q ∨ s) ∧ (r ∨ s):
```cpp
#include <dpll.hpp>

int main(void) {
    dpll::Formula formula({
        {"p", "q"},     
        {"!p", "r"},    
        {"!q", "!r"},    
        {"p", "!s"},       
        {"q", "s"},       
        {"r", "s"}         
    });

    dpll::Solver::solve(formula);
}
```

Which yields the following output:
```bash
Starting solver...
Formula: (p ∨ q) ∧ (¬p ∨ r) ∧ (¬q ∨ ¬r) ∧ (p ∨ ¬s) ∧ (q ∨ s) ∧ (r ∨ s)
Solver finished in 0ms
Formula is satisfiable!
Assignment:
s = true
r = true
q = false
p = true
```

### Arguments
| Parameter | Description | Default |
|-----------|-------------|---------|
| `formula` | Formula to find satisfying assignment for | Required |
| `useUnitProp` | Whether to use unit propagation | `true` |
| `usePureAssign` | Whether to use pure literal assignment | `true` |
| `silent` | Whether to print to stdout | `false` |
