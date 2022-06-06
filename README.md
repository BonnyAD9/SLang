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

## Comments
- use `//` for line comments
- use `/* */` for block comments (`/*/` is valid block comment)