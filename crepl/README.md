# M4 - crepl

## <TL; DR>

- Debug mode is defined in `crepl.h`. The submitted version turns that off.

- If a function calls a undefined function, the compilation will pass but you cannot calculate. 

  You can define that function later, and the calculation will pass then.

  For example, you can define

  ```C
  int f() { return g(); }
  int g() { return XXX; }
  ```

  and `f` will not work before `g` is defined.


## Main procedure

- There is an infinite loop in the main() function, keeps taking input, then calculate and output the result.
- After scanning the input, there is a helper function `precheck()` to determine whether the input is a function, an evaluation or `exit/quit`.
- If it is a function, `compile` will be called, then a child process is created to call `gcc` to compile the code into a shared library. Then the parent process will wait and then load the library using `RTLD_LAZY | RTLD_GLOBAL` options.
- If it is an evaluation, then a wrapper will wrap the code, then call `compile` to compile the function and create a child process to calculate the value, pass it to the parent using pipe. This is because if a call to undefined function is made, the child process will break, and the parent process can catch the error.

