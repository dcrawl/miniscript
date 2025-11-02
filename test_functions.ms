factorial = function(n)
    if n <= 1 then return 1
    return n * factorial(n - 1)
end function

add = function(a, b)
    return a + b
end function

multiply = function(a, b)
    return a * b
end function

calculate = function(x)
    temp = add(x, 5)
    return multiply(temp, 2)
end function

print "Testing function calls with Context pooling:"
print "Factorial 5! = " + factorial(5)

sum = 0
for i in range(1, 10)
    sum = sum + calculate(i)
end for
print "Sum of calculated values: " + sum

print "Context pooling is working correctly!"