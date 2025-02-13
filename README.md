# Davis–Putnam–Logemann–Loveland (DPLL) algorithm
DPLL single-header include in C++20

## Example Usage
Formula must be encoded in CNF

E.g. to solve the following SAT problem: (p ∨ q) ∧ (¬p ∨ r) ∧ (¬q ∨ ¬r)
```cpp
#include <Dpll.hpp>

int main(void) {
    dpll::Formula formula({
        {"p", "q"},     
        {"!p", "r"},    
        {"!q", "!r"},    
        {"p", "!s"},       
        {"q", "s"},       
        {"r", "s"}         
    });

    dpll::Dpll::solve(formula);
}
```

Which yields the following output:
```bash
Starting solver...
Formula: (p ∨ q) ∧ (¬p ∨ r) ∧ (¬q ∨ ¬r) ∧ (p ∨ ¬s) ∧ (q ∨ s) ∧ (r ∨ s)
Formula is satisfiable!
Assignment:
s = true
r = true
q = false
p = true
```