[set factorial' [def [/*int*/num /*int*/hold]
    [if [<= num 1]
        1
        [factorial' [- num 1] [* hold num]]
    ]
]]

/**
 * @param num number to calculate factorial
 * @returns result of the factorial
 */
[set factorial [def [/*/num]
    [factorial' num 1]
]]

[echo [factorial 7]]
// hmm