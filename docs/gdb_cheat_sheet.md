# GDB Cheat Sheet - Useful GDB Commands

## Breakpoints

- Set a breakpoint at a specific function:

  ```gdb
  (gdb) break function_name
  ```

- Set a breakpoint at line linenum in the current source file

  ```gdb
  (gdb) break linenum
  ```

- Delete a breakpoint:
  ```gdb
  (gdb) delete <breakpoint_number>
  ```

## Navigation

- Start execution:

  ```gdb
  (gdb) continue
  ```

- Step one line (C source level):

  ```gdb
  (gdb) step
  ```

- Execute next line without entering functions:

  ```gdb
  (gdb) next
  ```

- Move up the call stack (show the calling function)

```gdb
(gdb) up
```

- Move down the call stack to return to the current function

```gdb
(gdb) down
```

## Kill programm

```gdb
(gdb) kill
```

## Exiting GDB

```gdb
(gdb) quit
```
