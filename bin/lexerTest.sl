[sign factorial' [[int int] int]]
[sign pi float]

[struct complex [float real] [float imaginary]]

[struct node [* next] [_ item]]

/**
 * @param num number to calculate factorial
 * @returns result of the factorial
 */
[set factorial [def [/*/num]
    [factorial' num 1]
]]

[set factorial' [def [/*int*/num /*int*/hold]
    [if [<= num 1]
        1
        [factorial' [+ num -1] [* hold num]]
    ]
]]

[echo "\x1b[93mresult:\x1b[0m
" [factorial 9223372036854775807] struct]

[echo '-pi' -3.14159265358979323846264338327950288419716939937510]
[echo "numbers: " 0xFF 0b1001 36z10]
// hmm