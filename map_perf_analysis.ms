// Map Performance Analysis
// Tests current map implementation performance patterns

print "===== Map Performance Analysis ====="
print ""

// Test 1: Map Creation and Basic Operations
print "Test 1: Basic Map Operations"
testBasicOps = function()
	m = {}
	
	// Insert operations
	for i in range(1, 100)
		m["key" + str(i)] = i * 2
	end for
	
	// Lookup operations  
	sum = 0
	for i in range(1, 100)
		sum = sum + m["key" + str(i)]
	end for
	
	// Iteration
	count = 0
	for key in m.indexes
		count = count + 1
	end for
	
	return {"sum": sum, "count": count}
end function

result1 = testBasicOps
print "  Result: " + str(result1)
print ""

// Test 2: Hash Distribution Analysis
print "Test 2: Hash Distribution (collision patterns)"
testHashDistribution = function()
	// Test with similar keys (likely to cause collisions)
	m = {}
	
	// Similar string keys
	for i in range(1, 50)
		m["var" + str(i)] = i
		m["val" + str(i)] = i
	end for
	
	// Number keys  
	for i in range(1, 50)
		m[i] = "num" + str(i)
	end for
	
	return m.len
end function

result2 = testHashDistribution
print "  Map size with mixed keys: " + result2
print ""

// Test 3: Large Map Performance
print "Test 3: Large Map Scaling"
testLargeMap = function()
	m = {}
	
	// Build large map
	for i in range(1, 1000)
		key = "item_" + str(i)
		m[key] = {"id": i, "value": i * i, "name": "Item " + str(i)}
	end for
	
	// Random access pattern (worst case for cache)
	lookupCount = 0
	for i in range(1, 100)
		testKey = "item_" + str((i * 37) % 1000 + 1)  // pseudo-random
		if m.hasIndex(testKey) then
			lookupCount = lookupCount + 1
		end if
	end for
	
	return {"mapSize": m.len, "lookups": lookupCount}
end function

result3 = testLargeMap
print "  Result: " + str(result3)
print ""

// Test 4: Map vs List Performance Comparison
print "Test 4: Map vs List for Key-Value Storage"

testMapAccess = function()
	m = {}
	for i in range(1, 200)
		m["item" + str(i)] = i
	end for
	
	sum = 0
	for i in range(1, 200)
		sum = sum + m["item" + str(i)]
	end for
	return sum
end function

testListAccess = function()
	// Simulate map with list of [key, value] pairs
	lst = []
	for i in range(1, 200)
		lst.push(["item" + str(i), i])
	end for
	
	// Linear search for lookups
	sum = 0
	for i in range(1, 200)
		target = "item" + str(i)
		for item in lst
			if item[0] == target then
				sum = sum + item[1]
				break
			end if
		end for
	end for
	return sum
end function

mapSum = testMapAccess
listSum = testListAccess
print "  Map access sum: " + mapSum
print "  List search sum: " + listSum
print "  Results match: " + (mapSum == listSum)
print ""

// Test 5: Memory Usage Patterns
print "Test 5: Memory Usage Analysis"
testMemoryPatterns = function()
	maps = []
	
	// Create many small maps (tests allocation overhead)
	for i in range(1, 100)
		m = {}
		m["a"] = i
		m["b"] = i * 2
		m["c"] = i * 3
		maps.push(m)
	end for
	
	// Create one large map
	largemap = {}
	for i in range(1, 300)
		largemap["key" + str(i)] = i
	end for
	
	return {"smallMaps": maps.len, "largeMapSize": largemap.len}
end function

result5 = testMemoryPatterns
print "  Result: " + str(result5)
print ""

// Test 6: Key Type Performance
print "Test 6: Different Key Types"

testStringKeys = function()
	m = {}
	for i in range(1, 100)
		m["str_" + str(i)] = i
	end for
	return m.len
end function

testNumberKeys = function()
	m = {}
	for i in range(1, 100)
		m[i] = "value_" + str(i)
	end for
	return m.len
end function

stringResult = testStringKeys
numberResult = testNumberKeys
print "  String keys: " + stringResult
print "  Number keys: " + numberResult
print ""

print "===== Analysis Complete ====="
print ""
print "Current Map Implementation Observations:"
print "- Uses chaining hash table with fixed 251 buckets"
print "- Linear probing within chains for collision resolution"
print "- No dynamic resizing (fixed table size)"
print "- String and number keys both supported"
print "- Reference counting for memory management"
print ""
print "Potential optimization areas identified:"
print "1. Dynamic resizing for better load factors"  
print "2. More efficient hash functions"
print "3. Open addressing vs chaining"
print "4. Specialized storage for common key patterns"