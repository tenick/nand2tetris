# Branching commands
## label
- VM Command: $label <label>$
- Declares a label
## unconditional jump
- VM Command: $goto <label>$
- Literally just jump to the label
## conditional jump
- VM Command: $if-goto <label>$
- Guaranteed there's a boolean value on top of the stack
    - This boolean value comes from a logical/comparison command like eq, gt, and, etc.. just before this `if-goto <label>` command
- Use that boolean value, via $pop()$, then do the jump if value is `-1` (i.e. `true`) (fyi, `0` = `false`)

# Function commands
## call contract
- VM Command: $call functionName nArgs$
- save current function state in stack
    - this is done by pushing return address, lcl, arg, this, that onto the stack
- reposition ARG to $SP-5-nArgs$
- reposition LCL to SP
- jump to the start of the called function code entrypoint (using label)
- declare label just below the above instruction, that serves as the return address
## function contract
- VM Command: $function functionName nVars$
- declare label for this function's code entrypoint
- do `push constant 0` to the stack `nVars` times, to initialize the local segment for this function to all $0$
## return contract
- VM Command: $return$
- Can assume there's a return value on top of current stack
- let $endFrame = LCL$
- let $retAddr = *(endFrame-5)$
- $*ARG=pop()$
- $SP=ARG+1$
- restore the previous function state:
    - set LCL to `*(endFrame-4)`, ARG to `*(endFrame-3)`, etc...
- jump to the return address saved in `retAddr`
