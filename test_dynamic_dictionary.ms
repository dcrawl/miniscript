// Test Dynamic Dictionary Enhancement - Phase 1.1
// This test verifies that the dictionary can handle many entries efficiently
// with automatic resizing

print "Testing Dynamic Dictionary Performance..."
print "Phase 1 Enhancement: Dynamic Resizing Capability"
print "==========================================="

// Test 1: Basic functionality with small number of entries
d = {}
for i in range(1, 50)
    d[i] = "value" + i
end for
print "Test 1 - Basic functionality: " + d.len + " entries added"

// Test 2: Load the dictionary with many entries to trigger resizing
print
print "Test 2 - Heavy load test to trigger resizing..."
startTime = time

heavyDict = {}
// Add 2000 entries to trigger multiple resize operations
for i in range(1, 2001)
    key = "key_" + i
    value = "This is a test value for key " + i + " with some extra text to make it substantial"
    heavyDict[key] = value
    
    // Print progress every 500 entries
    if i % 500 == 0 then
        print "  Added " + i + " entries..."
    end if
end for

endTime = time
insertTime = endTime - startTime

print "Heavy load test completed:"
print "  Final size: " + heavyDict.len + " entries"
print "  Time taken: " + floor(insertTime * 1000 + 0.5) + " ms"
print "  Average time per insert: " + floor(insertTime * 1000 / 2000 * 100 + 0.5) / 100 + " ms"

// Test 3: Random access performance
print
print "Test 3 - Random access performance test..."
startTime = time

accessCount = 1000
hits = 0
for i in range(1, accessCount + 1)
    // Access random keys
    randomKey = "key_" + floor(rnd * 2000 + 1)
    if heavyDict.hasIndex(randomKey) then
        value = heavyDict[randomKey]
        hits = hits + 1
    end if
end for

endTime = time
accessTime = endTime - startTime

print "Random access test completed:"
print "  Lookups performed: " + accessCount
print "  Successful hits: " + hits
print "  Time taken: " + floor(accessTime * 1000 + 0.5) + " ms"
print "  Average time per lookup: " + floor(accessTime * 1000 / accessCount * 1000 + 0.5) / 1000 + " ms"

// Test 4: Test removal and table shrinking
print
print "Test 4 - Testing removal and table shrinking..."
startTime = time

originalSize = heavyDict.len
removed = 0

// Remove 75% of entries to test shrinking
for i in range(1, 1501)  // Remove first 1500 entries
    key = "key_" + i
    if heavyDict.hasIndex(key) then
        heavyDict.remove(key)
        removed = removed + 1
    end if
    
    if i % 500 == 0 then
        print "  Removed " + removed + " entries so far..."
    end if
end for

endTime = time
removeTime = endTime - startTime

print "Removal test completed:"
print "  Original size: " + originalSize + " entries"
print "  Entries removed: " + removed
print "  Final size: " + heavyDict.len + " entries"
print "  Time taken: " + floor(removeTime * 1000 + 0.5) + " ms"

// Test 5: Final verification
print
print "Test 5 - Final verification..."
finalTest = {}
for i in range(1, 100)
    finalTest["test_" + i] = i * i
end for

sum = 0
for key in finalTest.indexes
    sum = sum + finalTest[key]
end for

expectedSum = 0
for i in range(1, 100)
    expectedSum = expectedSum + (i * i)
end for

print "Sum of squares test:"
print "  Expected: " + expectedSum
print "  Actual:   " + sum
print "  Match:    " + (sum == expectedSum)

print
print "==========================================="
print "Dynamic Dictionary Enhancement Test Complete"
print
print "Key improvements from Phase 1.1:"
print "- Automatic table resizing based on load factor"
print "- Prime number table sizes for better distribution"  
print "- Memory-efficient rehashing during resize"
print "- Backward compatibility with existing API"
print "- Table shrinking to prevent memory waste"