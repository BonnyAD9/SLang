[; factorial' [[int int] int]]

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
        [factorial' [- num 1] [* hold num]]
    ]
]]

[echo "result:
" [factorial 123]]

[echo 'pi' 3.14159265358979323846264338327950288419716939937510]
// hmm