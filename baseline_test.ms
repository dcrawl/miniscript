// Simple test to verify basic map functionality
// This tests the current implementation before we implement dynamic dictionaries

print "Basic Map Functionality Test"
print "============================"

// Test 1: Basic operations
print "Test 1: Basic Operations"
m = {}
m["a"] = 1
m["b"] = 2
m[42] = "number key"

if m.hasIndex("a") and m.hasIndex("b") and m.hasIndex(42) then
    print "✓ Basic insertion and lookup works"
else
    print "✗ Basic operations failed"
end if

// Test 2: Iteration
print "Test 2: Iteration"
count = 0
for key in m.indexes
    count = count + 1
end for

if count == 3 then
    print "✓ Iteration works (found " + count + " keys)"
else
    print "✗ Iteration failed (found " + count + " keys, expected 3)"
end if

// Test 3: Larger map
print "Test 3: Larger Map (1000 entries)"
large_map = {}
for i in range(1000)
    large_map[i] = i * i
end for

// Verify some entries
test_keys = [0, 100, 500, 999]
all_correct = true
for key in test_keys
    expected = key * key
    if large_map[key] != expected then
        all_correct = false
        print "✗ Key " + key + " has wrong value"
    end if
end for

if all_correct then
    print "✓ Large map works correctly"
else
    print "✗ Large map has errors"
end if

// Test 4: Performance test (simple)
print "Test 4: Performance Test"
perf_map = {}

// Fill with 2000 entries
for i in range(2000)
    perf_map[i] = i
end for

// Access all entries (this should be slow with current fixed hash table)
sum = 0
for i in range(2000)
    sum = sum + perf_map[i]
end for

expected_sum = 2000 * 1999 / 2  // Sum formula: n(n-1)/2
if sum == expected_sum then
    print "✓ Performance test completed correctly (sum=" + sum + ")"
else
    print "✗ Performance test failed (sum=" + sum + ", expected=" + expected_sum + ")"
end if

print ""
print "Current implementation baseline complete."
print "With 2000+ entries, performance should degrade due to high load factor."
print "Load factor = 2000/251 = " + (2000/251) + " (should be < 0.75 for good performance)"