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
<img width="225" height="276" alt="2026-04-19-202353_hyprshot" src="https://github.com/user-attachments/assets/5e874c6b-f75a-4e92-b35a-37a1aa232888" />
Tape file (the ^ indicates the position of the head):
<img width="132" height="76" alt="2026-04-19-203444_hyprshot" src="https://github.com/user-attachments/assets/ade35a19-16be-46ca-9dd5-6ab07a323781" />
## Examples
Output of a 3 state 2 symbol busy beaver turing machine with all steps shown
<img width="241" height="1158" alt="image" src="https://github.com/user-attachments/assets/e291f47d-f2f4-48c4-865c-781633387e22" />
