factorial = function(n)
    if n <= 1 then return 1
    return n * factorial(n - 1)
end function

print "Testing recursive function with Context pooling:"
print "Factorial 5! = " + factorial(5)
print "Context pooling optimization complete!"