//[defined factorial' [[int int] int]]
//[defined pi float]

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

[echo "\x1b[93mresult:\x1b[0m" [factorial 9223372036854775807]]

[echo "-pi" -3.1415926535897932]
[echo "numbers: " 0xFF 0b1001 36z10]
// hmm