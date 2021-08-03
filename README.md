# Assembler for Route Language as used in GNC repositories

## Mnemonics:

### INTEGER:
Define an 16 bit signed integer\
Usage:
`INTEGER [name] [value]`
Example:
`INTEGER count 18`

### POINT:
Describes a waypoint in north-east-down coordinates\
Usage:
`POINT x y z`
Example:
`POINT 14.2 17 19.75`

### PRINT:
Print integer to the standard output\
Usage:
`PRINT [varname]`
Example:
`PRINT count`

### WHILE:
Start unlimited while loop\
Usage:
```
WHILE
..
..
ENDWHLIE
```

### WHILE_VAR
Start while loop that will run until the variable is non-zero\
Usage:
```
WHILE_VAR count
..
..
ENDWHILE
```

### ENDWHILE
Marks the end of a while or while_var loop\
Usage:
```
WHILE
..
..
ENDWHILE
```

### BREAK_WHILE
Breaks the current while loop\
Usage:
```
WHILE
..
BREAK_WHILE
..
END_WHILE
```

### FOR
Start a loop that will run for the count given\
Usage:
```
FOR [value]
..
ENDFOR
```
Example:
```
FOR 54
..
ENDFOR
```

### FOR_VAR
Start a for loop that will run for the count of the passed variable\
Usage:
```
FOR_VAR [varname]
..
ENDFOR
```
Example:
```
FOR_VAR count
..
ENDFOR
```

### IF_x
Start an if statement\
IF_Z will run if given variable is zero\
IF_NZ will run if given variable is non-zero\
IF_POS will run if given variable is > zero\
IF_NEG will run if given variable is < zero\
Usage:
```
IF_x [varname]
..
ENDIF
```
Example:
```
IF_Z count
..
ENDIF
```

### ENDIF
Terminate an if statement\
Usage:
```
IF_Z
..
ENDIF
```

### INCREMENT / DECREMENT
Perform respective operations on given variable\
Usage:
```
[operation] [varname]
```
Example:
```
DECREMENT count
```

### ADD / SUB / MUL / DIV
Perform respective operation on first two variables, and write back to final variable\
Usage:
```
[operation] [varname1] [varname2] [writebackvar]
```
Example:
```
ADD count othervar writeback
```

### ADD_ASSIGN / SUB_ASSIGN / MUL_ASSIGN / DIV_ASSIGN
Perform respective operation on given variable using given argument and write back\
Usage:
```
[operation] [varname] [coefficient]
```
Example
```
MUL_ASSIGN count 2
```

### END
Terminate program, Must be at end of every program\
Any code after an END directive is unreachable\
Usage:
```
..
..
END
```
