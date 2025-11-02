// Test Fibonacci calculation
n = 15
if n <= 1 then
    result = n
else
    a = 0
    b = 1
    for i in range(2, n)
        temp = a + b
        a = b
        b = temp
    end for
    result = b
end if
print result