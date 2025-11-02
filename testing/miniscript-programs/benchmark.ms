// MiniScript Performance Benchmark Suite
// This suite tests various aspects of the MiniScript interpreter performance
// Run with: ./miniscript benchmark.ms

// Utility function to time execution
timeit = function(name, iterations, func)
	print "Running " + name + " (" + iterations + " iterations)..."
	startTime = time
	for i in range(1, iterations)
		func
	end for
	endTime = time
	elapsed = endTime - startTime
	opsPerSecond = iterations / elapsed
	print "  " + name + ": " + round(elapsed,3) + "s total, " + round(opsPerSecond,0) + " ops/sec"
	return {"name": name, "elapsed": elapsed, "opsPerSecond": opsPerSecond}
end function

// Individual benchmark functions

// Arithmetic Operations Benchmark
arithmeticBench = function()
	a = 1.5
	b = 2.3
	c = 0
	for i in range(1, 100)
		c = a + b
		c = a - b  
		c = a * b
		c = a / b
		c = a ^ b
		c = a % b
	end for
end function

// Loop Performance Benchmark
loopBench = function()
	sum = 0
	for i in range(1, 1000)
		sum = sum + i
	end for
	return sum
end function

// Function Call Benchmark
funcCallBench = function()
	add = function(x, y)
		return x + y
	end function
	
	result = 0
	for i in range(1, 100)
		result = add(i, i + 1)
	end for
	return result
end function

// String Operations Benchmark
stringBench = function()
	s = "hello"
	result = ""
	for i in range(1, 100)
		result = s + " world"
		result = result + str(i)
		if result.len > 1000 then result = "reset"
	end for
	return result
end function

// List Operations Benchmark
listBench = function()
	lst = []
	for i in range(1, 100)
		lst.push(i)
		if lst.len > 50 then lst = []
	end for
	
	// List access
	sum = 0
	for i in range(0, lst.len - 1)
		sum = sum + lst[i]
	end for
	return sum
end function

// Map Operations Benchmark
mapBench = function()
	m = {}
	for i in range(1, 100)
		m[str(i)] = i * 2
	end for
	
	// Map access
	sum = 0
	for key in m.indexes
		sum = sum + m[key]
	end for
	return sum
end function

// Recursive Function Benchmark (Fibonacci)
fibBench = function()
	fibonacci = function(n)
		if n <= 1 then return n
		return fibonacci(n - 1) + fibonacci(n - 2)
	end function
	
	return fibonacci(20)
end function

// Nested Loop Benchmark
nestedLoopBench = function()
	sum = 0
	for i in range(1, 50)
		for j in range(1, 50)
			sum = sum + i * j
		end for
	end for
	return sum
end function

// Object-Oriented Style Benchmark
ooBench = function()
	Point = {}
	Point.x = 0
	Point.y = 0
	Point.distance = function()
		return sqrt(self.x^2 + self.y^2)
	end function
	Point.move = function(dx, dy)
		self.x = self.x + dx
		self.y = self.y + dy
	end function
	
	p = new Point
	for i in range(1, 100)
		p.move(1, 1)
		dist = p.distance
	end for
	return p.distance
end function

// Conditional Logic Benchmark
conditionalBench = function()
	result = 0
	for i in range(1, 100)
		if i % 2 == 0 then
			result = result + i
		else if i % 3 == 0 then
			result = result - i
		else
			result = result * 1.1
		end if
	end for
	return result
end function

// Main benchmark runner
print "===== MiniScript Performance Benchmark Suite ====="
print "Testing interpreter performance across various workloads"
print "Timestamp: " + str(time)
print ""

results = []

// Run benchmarks
results.push(timeit("Arithmetic Operations", 1000, @arithmeticBench))
results.push(timeit("Simple Loops", 100, @loopBench))  
results.push(timeit("Function Calls", 1000, @funcCallBench))
results.push(timeit("String Operations", 500, @stringBench))
results.push(timeit("List Operations", 500, @listBench))
results.push(timeit("Map Operations", 500, @mapBench))
results.push(timeit("Recursion (Fibonacci)", 10, @fibBench))
results.push(timeit("Nested Loops", 10, @nestedLoopBench))
results.push(timeit("Object-Oriented Style", 200, @ooBench))
results.push(timeit("Conditional Logic", 500, @conditionalBench))

print ""
print "===== Summary ====="
totalTime = 0
for result in results
	totalTime = totalTime + result.elapsed
end for
print "Total benchmark time: " + round(totalTime, 3) + "s"

// Find fastest and slowest operations
fastest = results[0]
slowest = results[0]
for result in results
	if result.opsPerSecond > fastest.opsPerSecond then fastest = result
	if result.opsPerSecond < slowest.opsPerSecond then slowest = result
end for

print "Fastest operation: " + fastest.name + " (" + round(fastest.opsPerSecond,0) + " ops/sec)"
print "Slowest operation: " + slowest.name + " (" + round(slowest.opsPerSecond,0) + " ops/sec)"
print "Performance ratio: " + round((fastest.opsPerSecond / slowest.opsPerSecond), 1) + ":1"