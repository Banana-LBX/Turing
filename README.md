# Turing
Interpreter for a language based on the turing machine.
## Usage
The interpreter requires a text file with the turing machine tape and a text file with the ruleset
```sh
turing tape.txt rules.txt
```
You can also pass flags into the interpreter
```sh
turing -a 1 tape.txt rules.txt # shows each step of the machine with a delay of 1 second
```
## Syntax
Rules file (based on the turing machine state diagram):
```txt
# [state]: [read]/[write]/[movement]/[next state]
# [movement]: L(left) R(right) .(stay)
q0: 0/1/R/q1 # there are inline comments too
q1: 0/0/R/q2
q2: 0/1/L/q2

q0: 1/1/R/qH
q1: 1/1/R/q1
q2: 1/1/L/q0
```
Tape file (the ^ indicates the position of the head):
```txt
000000
 ^
```
## Examples
Output of a 3 state 2 symbol busy beaver turing machine with all steps shown
<img width="241" height="1158" alt="image" src="https://github.com/user-attachments/assets/e291f47d-f2f4-48c4-865c-781633387e22" />
