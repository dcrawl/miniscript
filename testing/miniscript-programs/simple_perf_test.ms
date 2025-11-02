// Simple MiniScript Performance Test
// Uses iteration counting instead of timing

print "===== MiniScript Performance Tests ====="
print "Testing relative performance of operations"
print ""

// Simple performance test using loop counts
testOperation = function(name, iterations, operation)
	print "Testing: " + name + " (" + iterations + " iterations)"
	
	// Run the operation
	for i in range(1, iterations)
		operation
	end for
	
	print "  Completed " + iterations + " iterations"
	return iterations
end function

// Test functions
arithmeticTest = function()
	a = 3.14159
	b = 2.71828
	c = a + b
	c = a * b
	c = a / b
end function

stringTest = function()
	s = "hello"
	t = "world"
	result = s + " " + t
	len = result.len
end function

listTest = function()
	lst = [1, 2, 3, 4, 5]
	lst.push(6)
	val = lst[2]
	len = lst.len
end function

mapTest = function()
	m = {"a": 1, "b": 2}
	m["c"] = 3
	val = m["a"]
end function

functionCallTest = function()
	testFunc = function(x)
		return x * 2
	end function
	result = testFunc(5)
end function

loopTest = function()
	sum = 0
	for i in range(1, 100)
		sum = sum + i
	end for
end function

// Run tests with different iteration counts
baseIterations = 10000

print "Running performance tests..."
print ""

testOperation("Arithmetic Operations", baseIterations, @arithmeticTest)
testOperation("String Operations", baseIterations / 2, @stringTest)
testOperation("List Operations", baseIterations / 2, @listTest)
testOperation("Map Operations", baseIterations / 2, @mapTest)
testOperation("Function Calls", baseIterations / 5, @functionCallTest)
testOperation("Loops (nested)", baseIterations / 100, @loopTest)

print ""
print "Performance test completed!"
print ""

// Now test some specific bottlenecks
print "===== Specific Performance Analysis ====="
print ""

// Variable access patterns
print "Testing variable access patterns:"

// Local vs global variable access
globalVar = 42

localVarTest = function()
	localVar = 42
	for i in range(1, 1000)
		x = localVar
	end for
end function

globalVarTest = function()
	for i in range(1, 1000)
		x = globalVar
	end for
end function

print "Local variable access test..."
localVarTest
print "Global variable access test..."
globalVarTest

// Function call overhead
print ""
print "Testing function call overhead:"

emptyFunc = function()
end function

simpleFunc = function(x)
	return x
end function

mathFunc = function(x, y)
	return x * y + 1
end function

print "Empty function calls..."
for i in range(1, 1000)
	emptyFunc
end for

print "Simple function calls..."
for i in range(1, 1000)
	result = simpleFunc(i)
end for

print "Math function calls..."
for i in range(1, 1000)
	result = mathFunc(i, i + 1)
end for

// Data structure operations
print ""
print "Testing data structure operations:"

print "List growth..."
lst = []
for i in range(1, 1000)
	lst.push(i)
end for
print "Final list length: " + lst.len

print "Map growth..."
m = {}
for i in range(1, 1000)
	m[str(i)] = i
end for
print "Final map size: " + m.len

// String operations
print ""
print "Testing string operations:"

print "String concatenation..."
result = ""
for i in range(1, 100)
	result = result + str(i)
end for
print "Final string length: " + result.len

// Nested operations
print ""
print "Testing nested operations:"

print "Nested loops..."
count = 0
for i in range(1, 50)
	for j in range(1, 50)
		count = count + 1
	end for
end for
print "Total iterations: " + count

print "Nested function calls..."
level3 = function(x)
	return x + 1
end function

level2 = function(x)
	return level3(x) + 1
end function

level1 = function(x)
	return level2(x) + 1
end function

result = 0
for i in range(1, 100)
	result = result + level1(i)
end for
print "Nested call result: " + result

print ""
print "===== Analysis Complete ====="
print "This provides a baseline for MiniScript interpreter performance"