// Real-World MiniScript Benchmarks
// Tests performance on realistic scenarios

import "stringUtil"
import "mathUtil"

benchmark = function(name, func)
	print "Running: " + name
	startTime = time
	result = func
	endTime = time
	elapsed = endTime - startTime
	print "  Completed in: " + round(elapsed, 4) + "s"
	resultStr = str(result)
	if resultStr.len > 50 then
		print "  Result: " + resultStr[0 : 50] + "..."
	else
		print "  Result: " + resultStr
	end if
	print ""
	return elapsed
end function

print "===== Real-World MiniScript Benchmarks ====="
print ""

// 1. Data Processing - CSV-like operations
csvProcessing = function
	// Simulate processing a dataset
	data = []

	// Generate sample data
	for i in range(1, 1000)
		record = {
			"id": i,
			"name": "Person" + str(i),
			"age": 20 + (i % 50),
			"score": (i * 17) % 100,
			"active": (i % 3 == 0),
		}
		data.push record
	end for

	// Process data - filter and transform
	filtered = []
	totalScore = 0
	activeCount = 0

	for record in data
		if record.age >= 25 and record.score > 50 then
			record.category = "high"
			filtered.push record
		end if

		totalScore += record.score
		if record.active then activeCount += 1
	end for

	avgScore = totalScore / data.len

	return {
		"totalRecords": data.len,
		"filteredRecords": filtered.len,
		"averageScore": round(avgScore, 2),
		"activeCount": activeCount,
	}
end function

// 2. Text Processing - Simple parser
textProcessing = function
	// Simulate parsing a structured text format
	text = []
	for i in range(1, 200)
		line = "item" + str(i) + ": value=" + str(i * 3) + ", type=" + [
			"A",
			"B",
			"C",
		][i % 3] + ", enabled=true"
		text.push line
	end for

	parsed = []
	for line in text
		// Simple parsing
		parts = line.split(":")
		if parts.len == 2 then
			name = parts[0].trim
			valueStr = parts[1].trim

			// Parse key-value pairs
			pairs = valueStr.split(",")
			item = { "name": name }

			for pair in pairs
				kv = pair.split("=")
				if kv.len == 2 then
					key = kv[0].trim
					value = kv[1].trim

					// Try to convert to number
					if value.val != null then
						item[key] = value.val
					else
						item[key] = value
					end if
				end if
			end for

			parsed.push item
		end if
	end for

	return parsed.len
end function

// 3. Game Logic Simulation
gameSimulation = function
	// Simulate a simple game with entities and physics
	Entity = {
		"x": 0,
		"y": 0,
		"vx": 0,
		"vy": 0,
	}
	gameSimulation.update = function(dt)
		self.x += self.vx * dt
		self.y += self.vy * dt

		// Simple friction
		self.vx *= 0.98
		self.vy *= 0.98

		// Boundary wrapping
		if self.x > 100 then self.x = 0
		if self.x < 0 then self.x = 100
		if self.y > 100 then self.y = 0
		if self.y < 0 then self.y = 100

    wait(0.01)
	end function

	gameSimulation.distanceTo = function(other)
		dx = self.x - other.x
		dy = self.y - other.y
		return sqrt(dx * dx + dy * dy)
	end function

	// Create entities
	entities = []
	for i in range(1, 100)
		entity = new Entity
		entity.x = rnd * 100
		entity.y = rnd * 100
		entity.vx = (rnd - 0.5) * 20
		entity.vy = (rnd - 0.5) * 20
		entities.push entity
	end for

	// Simulate game steps
	dt = 0.016 // 60 FPS
	collisions = 0

	for step in range(1, 120) // 2 seconds at 60 FPS
		// Update entities
		for entity in entities
			entity.update dt
		end for

		// Check for collisions (simple distance check)
		for i in range(0, entities.len - 1)
			for j in range(i + 1, entities.len - 1)
				if entities[i].distanceTo(entities[j]) < 5 then
					collisions += 1
				end if
			end for
		end for
	end for

	return {
		"entities": entities.len,
		"steps": 120,
		"collisions": collisions,
	}
end function

// 4. Mathematical Computing
mathComputation = function
	// Monte Carlo estimation of π
	samples = 10000
	inside = 0

	for i in range(1, samples)
		x = rnd * 2 - 1 // -1 to 1
		y = rnd * 2 - 1 // -1 to 1

		if x * x + y * y <= 1 then
			inside += 1
		end if
	end for

	piEstimate = 4 * inside / samples

	// Matrix operations
	matrixA = []
	matrixB = []
	size = 20

	// Create matrices
	for i in range(0, size - 1)
		rowA = []
		rowB = []
		for j in range(0, size - 1)
			rowA.push rnd
			rowB.push rnd
		end for
		matrixA.push rowA
		matrixB.push rowB
	end for

	// Matrix multiplication (simplified)
	result = []
	for i in range(0, size - 1)
		row = []
		for j in range(0, size - 1)
			sum = 0
			for k in range(0, size - 1)
				sum += matrixA[i][k] * matrixB[k][j]
			end for
			row.push sum
		end for
		result.push row
	end for

	return {
		"piEstimate": round(piEstimate, 4),
		"matrixSize": size,
		"matrixSum": round(result[0][0], 4),
	}
end function

// 5. Tree/Recursive Data Structure Operations
treeOperations = function
	// Binary tree implementation
	TreeNode = {
		"value": 0,
		"left": null,
		"right": null,
	}
	TreeNode.insert = function(val)
		if val < self.value then
			if self.left == null then
				self.left = new TreeNode
				self.left.value = val
			else
				self.left.insert val
			end if
		else
			if self.right == null then
				self.right = new TreeNode
				self.right.value = val
			else
				self.right.insert val
			end if
		end if
	end function

	TreeNode.search = function(val)
		if val == self.value then return true
		if val < self.value and self.left != null then
			return self.left.search(val)
		end if
		if val > self.value and self.right != null then
			return self.right.search(val)
		end if
		return false
	end function

	TreeNode.inorder = function
		result = []
		if self.left != null then result += self.left.inorder
		result.push self.value
		if self.right != null then result += self.right.inorder
		return result
	end function

	// Create and populate tree
	root = new TreeNode
	root.value = 50

	values = [
		25,
		75,
		10,
		30,
		60,
		80,
		5,
		15,
		27,
		35,
		55,
		65,
		77,
		85,
	]
	for val in values
		root.insert val
	end for

	// Perform operations
	searchCount = 0
	for i in range(1, 100)
		testVal = floor(rnd * 100)
		if root.search(testVal) then
			searchCount += 1
		end if
	end for

	ordered = root.inorder

	return {
		"treeSize": values.len + 1,
		"searchHits": searchCount,
		"inorderSize": ordered.len,
	}
end function

// Run all benchmarks
totalTime = 0
results = {}

results.csvProcessing = benchmark("Data Processing (CSV-like)", @csvProcessing)
totalTime += results.csvProcessing

results.textProcessing = benchmark("Text Processing (Parser)", @textProcessing)
totalTime += results.textProcessing

results.gameSimulation = benchmark("Game Logic Simulation", @gameSimulation)
totalTime += results.gameSimulation

results.mathComputation = benchmark("Mathematical Computing", @mathComputation)
totalTime += results.mathComputation

results.treeOperations = benchmark("Tree/Recursive Operations", @treeOperations)
totalTime += results.treeOperations

print "===== Real-World Benchmark Summary ====="
print "Total execution time: " + round(totalTime, 3) + "s"

// Find relative performance
fastest = null
slowest = null
for key in results.keys
	time = results[key]
	if fastest == null or time < results[fastest] then fastest = key
	if slowest == null or time > results[slowest] then slowest = key
end for

print "Fastest scenario: " + fastest + " (" + round(results[fastest], 3) + "s)"
print "Slowest scenario: " + slowest + " (" + round(results[slowest], 3) + "s)"

if results[slowest] > 0 then
	ratio = results[slowest] / results[fastest]
	print "Performance spread: " + round(ratio, 1) + ":1"
end if