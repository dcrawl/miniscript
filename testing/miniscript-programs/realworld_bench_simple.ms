// Real-World MiniScript Benchmarks (Simplified)
// Tests performance on realistic scenarios

import "dateTime"

benchmark = function(name, func)
	print "Running: " + name
	startTime = dateTime.now
	result = func
	endTime = dateTime.now
	elapsed = endTime - startTime
	print "  Completed in: " + round(elapsed, 4) + "s"
	resultStr = str(result)
	if resultStr.len > 50 then
		print "  Result: " + resultStr[0:50] + "..."
	else
		print "  Result: " + resultStr
	end if
	print ""
	return elapsed
end function

print "===== Real-World MiniScript Benchmarks ====="
print ""

// 1. Data Processing - CSV-like operations
csvProcessing = function()
	// Simulate processing a dataset
	data = []
	
	// Generate sample data
	for i in range(1, 1000)
		record = {}
		record.id = i
		record.name = "Person" + str(i)
		record.age = 20 + (i % 50)
		record.score = (i * 17) % 100
		record.active = (i % 3 == 0)
		data.push(record)
	end for
	
	// Process data - filter and transform
	filtered = []
	totalScore = 0
	activeCount = 0
	
	for record in data
		if record.age >= 25 and record.score > 50 then
			record.category = "high"
			filtered.push(record)
		end if
		
		totalScore = totalScore + record.score
		if record.active then activeCount = activeCount + 1
	end for
	
	avgScore = totalScore / data.len
	
	return "Processed " + data.len + " records, " + filtered.len + " filtered, avg score: " + round(avgScore, 2)
end function

// 2. Text Processing - Simple parser
textProcessing = function()
	// Simulate parsing a structured text format
	text = []
	for i in range(1, 200)
		line = "item" + str(i) + ": value=" + str(i * 3) + ", type=" + ["A", "B", "C"][i % 3] + ", enabled=true"
		text.push(line)
	end for
	
	parsed = []
	for line in text
		// Simple parsing
		parts = line.split(":")
		if parts.len == 2 then
			name = parts[0]
			valueStr = parts[1]
			
			// Parse key-value pairs
			pairs = valueStr.split(",")
			item = {}
			item.name = name
			
			for pair in pairs
				kv = pair.split("=")
				if kv.len == 2 then
					key = kv[0]
					value = kv[1]
					
					// Simple assignment
					if key == " value" then
						item.value = value.val
					else if key == " type" then
						item.type = value
					else if key == " enabled" then
						item.enabled = (value == "true")
					end if
				end if
			end for
			
			parsed.push(item)
		end if
	end for
	
	return "Parsed " + parsed.len + " items"
end function

// 3. Mathematical Computing
mathComputation = function()
	// Monte Carlo estimation of π
	samples = 5000
	inside = 0
	
	for i in range(1, samples)
		x = rnd * 2 - 1  // -1 to 1
		y = rnd * 2 - 1  // -1 to 1
		
		if x*x + y*y <= 1 then
			inside = inside + 1
		end if
	end for
	
	piEstimate = 4.0 * inside / samples
	
	// Simple matrix operations
	matrix = []
	size = 15
	
	// Create matrix
	for i in range(0, size - 1)
		row = []
		for j in range(0, size - 1)
			row.push(rnd * 10)
		end for
		matrix.push(row)
	end for
	
	// Calculate sum and average
	totalSum = 0
	for i in range(0, size - 1)
		for j in range(0, size - 1)
			totalSum = totalSum + matrix[i][j]
		end for
	end for
	
	average = totalSum / (size * size)
	
	return "π estimate: " + round(piEstimate, 4) + ", Matrix avg: " + round(average, 3)
end function

// 4. String and List Processing
stringListProcessing = function()
	// Generate text data
	words = ["the", "quick", "brown", "fox", "jumps", "over", "lazy", "dog", "and", "runs", "fast"]
	sentences = []
	
	for i in range(1, 100)
		sentence = []
		wordCount = 3 + (i % 8)
		for j in range(1, wordCount)
			sentence.push(words[j % words.len])
		end for
		sentences.push(sentence.join(" "))
	end for
	
	// Process sentences
	totalWords = 0
	longSentences = 0
	wordFreq = {}
	
	for sentence in sentences
		words = sentence.split(" ")
		totalWords = totalWords + words.len
		
		if words.len > 6 then longSentences = longSentences + 1
		
		// Count word frequency
		for word in words
			if wordFreq.hasIndex(word) then
				wordFreq[word] = wordFreq[word] + 1
			else
				wordFreq[word] = 1
			end if
		end for
	end for
	
	avgWordsPerSentence = totalWords / sentences.len
	uniqueWords = wordFreq.len
	
	return "Processed " + sentences.len + " sentences, " + totalWords + " words, " + uniqueWords + " unique"
end function

// 5. Nested Data Structure Operations
nestedDataProcessing = function()
	// Create hierarchical data structure
	departments = []
	
	for deptId in range(1, 10)
		dept = {}
		dept.id = deptId
		dept.name = "Department " + str(deptId)
		dept.employees = []
		
		// Add employees to department
		empCount = 5 + (deptId % 15)
		for empId in range(1, empCount)
			emp = {}
			emp.id = empId
			emp.name = "Employee " + str(empId)
			emp.salary = 30000 + (empId * 1000) + (deptId * 500)
			emp.projects = []
			
			// Add projects to employee
			projCount = 1 + (empId % 4)
			for projId in range(1, projCount)
				project = {}
				project.id = projId
				project.name = "Project " + str(projId)
				project.hours = 10 + (projId * 5) + (empId % 10)
				emp.projects.push(project)
			end for
			
			dept.employees.push(emp)
		end for
		
		departments.push(dept)
	end for
	
	// Process the data
	totalEmployees = 0
	totalSalary = 0
	totalProjects = 0
	totalHours = 0
	
	for dept in departments
		totalEmployees = totalEmployees + dept.employees.len
		
		for emp in dept.employees
			totalSalary = totalSalary + emp.salary
			totalProjects = totalProjects + emp.projects.len
			
			for project in emp.projects
				totalHours = totalHours + project.hours
			end for
		end for
	end for
	
	avgSalary = totalSalary / totalEmployees
	avgProjectsPerEmp = totalProjects / totalEmployees
	avgHoursPerProject = totalHours / totalProjects
	
	return "Departments: " + departments.len + ", Employees: " + totalEmployees + ", Avg salary: " + round(avgSalary, 0)
end function

// Run all benchmarks
totalTime = 0
results = {}

results.csvProcessing = benchmark("Data Processing (CSV-like)", @csvProcessing)
totalTime = totalTime + results.csvProcessing

results.textProcessing = benchmark("Text Processing (Parser)", @textProcessing)
totalTime = totalTime + results.textProcessing

results.mathComputation = benchmark("Mathematical Computing", @mathComputation)
totalTime = totalTime + results.mathComputation

results.stringListProcessing = benchmark("String/List Processing", @stringListProcessing)
totalTime = totalTime + results.stringListProcessing

results.nestedDataProcessing = benchmark("Nested Data Processing", @nestedDataProcessing)
totalTime = totalTime + results.nestedDataProcessing

print "===== Real-World Benchmark Summary ====="
print "Total execution time: " + round(totalTime, 3) + "s"

// Find relative performance
fastest = null
slowest = null
for key in results.indexes
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