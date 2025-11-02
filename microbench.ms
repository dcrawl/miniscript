// MiniScript Micro-Benchmarks
// Focus on interpreter-level operations to identify bottlenecks

// Precision timing utility
precisionTime = function(name, iterations, func)
	print "Micro-benchmark: " + name
	
	// Warm up
	for i in range(1, 100)
		func
	end for
	
	startTime = time
	for i in range(1, iterations)
		func
	end for
	endTime = time
	elapsed = endTime - startTime
	
	avgTime = (elapsed * 1000000) / iterations  // microseconds per operation
	print "  " + iterations + " iterations in " + round(elapsed, 4) + "s"
	print "  Average: " + round(avgTime, 3) + " μs per operation"
	print "  Rate: " + round((iterations / elapsed), 0) + " ops/sec"
	print ""
	return avgTime
end function

print "===== MiniScript Micro-Benchmarks ====="
print "Measuring individual operation performance"
print ""

// Variable access benchmarks
localVarAccess = function()
	x = 42
	y = x
end function

globalVar = 100
globalVarAccess = function()
	y = globalVar
end function

// Arithmetic operation benchmarks
intAddition = function()
	result = 1 + 2
end function

floatAddition = function()
	result = 1.5 + 2.7
end function

multiplication = function()
	result = 3.14159 * 2.71828
end function

division = function()
	result = 22.0 / 7.0
end function

modulo = function()
	result = 17 % 5
end function

power = function()
	result = 2 ^ 10
end function

// String operations
stringConcat = function()
	result = "hello" + " world"
end function

stringLength = function()
	s = "test string"
	len = s.len
end function

stringIndex = function()
	s = "abcdefghijk"
	char = s[5]
end function

// List operations
listCreation = function()
	lst = [1, 2, 3, 4, 5]
end function

listAppend = function()
	lst = [1, 2, 3]
	lst.push(4)
end function

listAccess = function()
	lst = [10, 20, 30, 40, 50]
	val = lst[2]
end function

listLength = function()
	lst = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
	len = lst.len
end function

// Map operations
mapCreation = function()
	m = {"a": 1, "b": 2, "c": 3}
end function

mapAccess = function()
	m = {"key": "value", "number": 42}
	val = m["key"]
end function

mapAssignment = function()
	m = {}
	m["new_key"] = "new_value"
end function

// Function call overhead
emptyFunction = function()
end function

functionCall = function()
	emptyFunction
end function

// Simple math function
mathFunc = function(x, y)
	return x * y + 1
end function

mathFunctionCall = function()
	result = mathFunc(3, 4)
end function

// Conditional operations
simpleIf = function()
	x = 5
	if x > 3 then
		y = x
	end if
end function

comparison = function()
	result = (5 > 3)
end function

// Loop operations
emptyLoop = function()
	for i in range(1, 10)
	end for
end function

// Type checking
typeCheck = function()
	x = 42
	isNumber = (x isa number)
end function

// Run micro-benchmarks
print "=== Variable Access ==="
precisionTime("Local variable access", 100000, @localVarAccess)
precisionTime("Global variable access", 100000, @globalVarAccess)

print "=== Arithmetic Operations ==="
precisionTime("Integer addition", 100000, @intAddition)
precisionTime("Float addition", 100000, @floatAddition)
precisionTime("Multiplication", 100000, @multiplication)
precisionTime("Division", 100000, @division)
precisionTime("Modulo", 100000, @modulo)
precisionTime("Power", 50000, @power)

print "=== String Operations ==="
precisionTime("String concatenation", 50000, @stringConcat)
precisionTime("String length", 100000, @stringLength)
precisionTime("String indexing", 100000, @stringIndex)

print "=== List Operations ==="
precisionTime("List creation", 50000, @listCreation)
precisionTime("List append", 50000, @listAppend)
precisionTime("List access", 100000, @listAccess)
precisionTime("List length", 100000, @listLength)

print "=== Map Operations ==="
precisionTime("Map creation", 50000, @mapCreation)
precisionTime("Map access", 100000, @mapAccess)
precisionTime("Map assignment", 50000, @mapAssignment)

print "=== Function Calls ==="
precisionTime("Empty function call", 50000, @functionCall)
precisionTime("Math function call", 50000, @mathFunctionCall)

print "=== Control Flow ==="
precisionTime("Simple if statement", 100000, @simpleIf)
precisionTime("Comparison operation", 100000, @comparison)
precisionTime("Empty loop (10 iter)", 10000, @emptyLoop)

print "=== Type Operations ==="
precisionTime("Type checking (isa)", 100000, @typeCheck)

print "Micro-benchmarks complete!"