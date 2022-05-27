[! factorial' [[int int] int]]

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
" [factorial 7]]
// hmm