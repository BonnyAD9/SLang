[set factorial' [def [num hold]
    [if [<= num 1]
        1
        [factorial' [- num 1] [* hold num]]
    ]
]]

[set factorial [def [num]
    [factorial' num 1]
]]

[echo [factorial 7]]