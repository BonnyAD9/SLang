# SLang
Simple language inspired by scheme

## Example
file.sl:
```Scheme
[println "Hello World"]
```
cli:
```shell
> slang file.sl
Hello World
```
## Features
- Runing builtin functions
- Comments

## TODO
- [X] add runtime errors
- [X] add basic aritmetic functions (+, -, *, /, %)
- [ ] add lazy evaluation
- [ ] add `if` function
- [ ] ability to set variables
- [ ] ability to create functions
- [ ] add `do` function
- [ ] local set
- [ ] ability to create structures
- [ ] pointers
- [ ] ability to create function signatures
- [ ] ability to import another file
- [ ] llvm based compiler

## Builtin functions
- `print` prints its arguments to the screen
- `println` prints its arguments to the screen and appends newline
- `+` sums variable number of arguments of type bool, int or float
- `-` negates single argument or subtracts second argument from the first (works with bool, int and float)
- `*` multiplies variable number of arguments of type bool, int or float
- `/` divides the first argument by the second argument (works with int and float)
- `%` performs a modulo with two int arguments

### Escape sequences
(in string and char literals)
- `\0` null character
- `\n` newline
- `\r` carrige return
- `\t` tab
- `\x` hex char
    - there is no length limit
    - `\x` followed by no hex character is same as `\0`
    - `\x68656c6c6f` is same as `hello`
    - sequence can by optionally ended with `;`
    - `\x68656c6c;6f` is same as `hell6f`
- any other character directly following `\` is interpreted literally
    - it is not recommended to use this for other characters than`\'`, `\"` and `\\` (new escape sequences may be added later)

## Comments
- use `//` for line comments
- use `/* */` for block comments (`/*/` is valid block comment)
