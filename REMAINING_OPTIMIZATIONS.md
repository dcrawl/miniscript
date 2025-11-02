# Remaining High-Impact Optimizations Implementation Plan

Based on the OPTIMIZATION_ROADMAP.md analysis and current project status, here are the most valuable remaining optimizations:

## 🎯 **Priority 1: Value System Optimization (Immediate Impact)**

### **1.1 NaN Boxing Implementation**
**Current Status**: Not implemented
**Expected Impact**: 20-30% performance improvement + 50% memory reduction
**Difficulty**: High
**Timeline**: 2-3 weeks

#### **Problem Analysis**
```cpp
// Current Value class: 16-24 bytes per Value
class Value {
    ValueType type;           // 1 byte (but padded to 4)
    bool noInvoke;           // 1 byte  
    LocalOnlyMode localOnly; // 1 byte
    // Padding: 1 byte
    union {                  // 8 bytes
        double number;
        RefCountedStorage* ref;
        int tempNum;
    } data;
    // Total: 16 bytes minimum, often 24 with alignment
};
```

#### **Proposed NaN Boxing Solution**
```cpp
// NaN-boxed Value: 8 bytes total
class NanBoxedValue {
    union {
        double number;      // Direct storage for numbers (most common)
        uint64_t bits;      // Tagged pointer for objects
    } data;
    
    // NaN bit patterns for type encoding:
    // Numbers: Standard IEEE 754 (no tagging needed)
    // Objects: 0x7FF8 + type_bits + pointer
    
    static constexpr uint64_t NAN_MASK = 0x7FF8000000000000ULL;
    static constexpr uint64_t TYPE_MASK = 0x0007000000000000ULL;
    
    enum TypeTag : uint64_t {
        STRING_TAG   = 0x0001000000000000ULL,
        MAP_TAG      = 0x0002000000000000ULL,
        LIST_TAG     = 0x0003000000000000ULL,
        FUNCTION_TAG = 0x0004000000000000ULL,
    };
    
    bool is_number() const { 
        return (data.bits & NAN_MASK) != NAN_MASK; 
    }
    
    double as_number() const {
        return data.number;
    }
    
    RefCountedStorage* as_object() const {
        return reinterpret_cast<RefCountedStorage*>(
            data.bits & ~(NAN_MASK | TYPE_MASK)
        );
    }
};
```

#### **Benefits**
- **50% memory reduction**: 8 bytes vs 16-24 bytes per Value
- **Better cache utilization**: More Values fit in cache lines
- **Faster number operations**: Direct access without union overhead
- **Reduced memory traffic**: Half the data movement for Value copies

### **1.2 Implementation Strategy**
1. **Phase A**: Create `NanBoxedValue` class in parallel with existing `Value`
2. **Phase B**: Add conversion functions and compatibility layer
3. **Phase C**: Gradual migration of core operations
4. **Phase D**: Performance validation and full integration

---

## 🎯 **Priority 2: Type-Specialized Instructions (High Impact, Lower Risk)**

### **2.1 Specialized TAC Operations**
**Current Status**: Generic operations only
**Expected Impact**: 40-60% improvement on arithmetic/string operations
**Difficulty**: Medium-High
**Timeline**: 3-4 weeks

#### **Current TAC Analysis**
```cpp
// Most common operations from profiling:
1. Arithmetic: +, -, *, / (30% of operations)
2. Map access: [] operator (25% of operations)  
3. Variable assignment (20% of operations)
4. String operations (15% of operations)
5. Comparisons (10% of operations)
```

#### **Proposed Specialized Instructions**
```cpp
enum class SpecializedOp {
    // Arithmetic (type-known at compile time)
    ADD_NUM_NUM,     // Both operands are numbers
    ADD_STR_STR,     // String concatenation
    ADD_NUM_STR,     // Number + string conversion
    MUL_NUM_NUM,     // Numeric multiplication
    
    // Variable access (scope-optimized)
    LOAD_LOCAL_0,    // Load local variable slot 0
    LOAD_LOCAL_1,    // Load local variable slot 1
    LOAD_GLOBAL_CACHED, // Cached global variable lookup
    
    // Map operations (key-type optimized)
    MAP_GET_STR_LITERAL,  // map["literal"] - string known at compile time
    MAP_GET_COMPUTED,     // map[expr] - computed key
    MAP_SET_STR_LITERAL,  // map["literal"] = value
    
    // Control flow (condition-optimized)
    JUMP_IF_NUMBER_TRUE,  // if (number_expr)
    JUMP_IF_BOOL_TRUE,    // if (bool_expr)
};
```

#### **Type Inference Engine**
```cpp
class SimpleTypeInference {
    enum InferredType { UNKNOWN, NUMBER, STRING, MAP, LIST };
    
    struct VariableInfo {
        InferredType type = UNKNOWN;
        bool is_literal = false;
        Value literal_value;
        int assignment_count = 0;
    };
    
    std::unordered_map<String, VariableInfo> vars;
    
public:
    void analyze_function(FunctionStorage* func) {
        // Single forward pass
        for (TACLine& line : func->code) {
            infer_line_types(line);
            specialize_if_possible(line);
        }
    }
    
private:
    void specialize_if_possible(TACLine& line) {
        if (line.op == Op::APlusB) {
            InferredType ltype = get_type(line.lhs);
            InferredType rtype = get_type(line.rhs);
            
            if (ltype == NUMBER && rtype == NUMBER) {
                line.op = Op::ADD_NUM_NUM;  // Specialized version
            } else if (ltype == STRING && rtype == STRING) {
                line.op = Op::ADD_STR_STR;
            }
        }
    }
};
```

### **2.2 Fast Path Implementation**
```cpp
// Enhanced TACLine evaluation with specialization
Value TACLine::EvaluateSpecialized(Context* context) {
    switch (op) {
    case Op::ADD_NUM_NUM:
        // Skip type checks - we know both are numbers
        return Value(lhs.FloatValue() + rhs.FloatValue());
        
    case Op::ADD_STR_STR:
        // Skip type checks - we know both are strings  
        return Value(lhs.ToString() + rhs.ToString());
        
    case Op::LOAD_LOCAL_0:
        // Direct slot access - no map lookup
        return context->locals[0];
        
    case Op::MAP_GET_STR_LITERAL:
        // Cached string key lookup
        return fast_map_get_string(lhs, cached_key);
        
    default:
        // Fallback to generic evaluation
        return EvaluateGeneric(context);
    }
}
```

---

## 🎯 **Priority 3: Advanced Map Optimizations (Medium-High Impact)**

### **3.1 Robin Hood Hashing Implementation**
**Current Status**: Separate chaining hash table
**Expected Impact**: 40-60% improvement on map operations
**Difficulty**: High
**Timeline**: 3-4 weeks

#### **Current Map Bottlenecks**
- **Poor cache locality**: Pointer chasing through hash chains
- **Variable access time**: O(1) average, O(n) worst case per bucket
- **Memory fragmentation**: Many small HashMapEntry allocations

#### **Robin Hood Hashing Solution**
```cpp
template<typename K, typename V>
class RobinHoodDictionary {
    struct Entry {
        K key;
        V value;
        uint16_t psl;  // Probe Sequence Length
        uint8_t hash_upper; // Upper bits for quick comparison
        bool occupied;
    };
    
    Entry* table;
    size_t capacity;
    size_t size;
    static constexpr size_t MAX_PSL = 64;  // Limit probe distance
    
public:
    V* find(const K& key) {
        size_t hash = hash_key(key);
        uint8_t hash_upper = hash >> 56;
        
        for (size_t psl = 0; psl < MAX_PSL; ++psl) {
            size_t index = (hash + psl) & (capacity - 1);
            Entry& entry = table[index];
            
            if (!entry.occupied || entry.psl < psl) {
                return nullptr;  // Not found
            }
            
            if (entry.hash_upper == hash_upper && entry.key == key) {
                return &entry.value;  // Found
            }
        }
        return nullptr;
    }
};
```

#### **Benefits**
- **Better cache locality**: Linear probing vs pointer chasing
- **Predictable performance**: Bounded probe distance
- **Memory efficiency**: No separate chain allocations
- **SIMD potential**: Can vectorize key comparisons

---

## 🎯 **Priority 4: Context and Memory Optimizations (Medium Impact)**

### **4.1 Context Pooling System**
**Current Status**: New/delete for each function call
**Expected Impact**: 15-25% improvement on function-heavy code
**Difficulty**: Medium
**Timeline**: 1-2 weeks

```cpp
class ContextPool {
    std::vector<std::unique_ptr<Context>> available;
    std::mutex pool_mutex;  // For thread safety
    
public:
    Context* acquire() {
        std::lock_guard<std::mutex> lock(pool_mutex);
        
        if (available.empty()) {
            return new Context();
        }
        
        Context* ctx = available.back().release();
        available.pop_back();
        ctx->reset();  // Clear previous state
        return ctx;
    }
    
    void release(Context* ctx) {
        std::lock_guard<std::mutex> lock(pool_mutex);
        ctx->clear();
        available.emplace_back(ctx);
    }
};
```

### **4.2 String Interning for Literals**
**Expected Impact**: 10-15% improvement on string-heavy code
**Implementation**: Cache frequently used string literals

---

## 📊 **Implementation Priority Matrix**

| Optimization | Impact | Difficulty | Timeline | Risk | Priority |
|-------------|--------|------------|----------|------|----------|
| NaN Boxing | Very High (30%+ improvement) | High | 2-3 weeks | Medium | **1** |
| Type Specialization | High (40-60%) | Medium-High | 3-4 weeks | Medium | **2** |
| Robin Hood Hashing | High (40-60% maps) | High | 3-4 weeks | Medium | **3** |
| Context Pooling | Medium (15-25%) | Medium | 1-2 weeks | Low | **4** |
| String Interning | Low-Medium (10-15%) | Low | 1 week | Low | **5** |

## 🎯 **Recommended Implementation Order**

### **Phase A (Immediate - 4 weeks)**
1. **Context Pooling** (1-2 weeks) - Low risk, immediate benefit
2. **Type-Specialized Instructions** (3-4 weeks) - High impact, builds on existing fast paths

### **Phase B (Short-term - 6 weeks)**  
3. **NaN Boxing** (2-3 weeks) - Massive memory and performance improvement
4. **Robin Hood Hashing** (3-4 weeks) - Major map performance boost

### **Phase C (Medium-term - 2 weeks)**
5. **String Interning** (1 week) - Polish and complete the optimization suite
6. **Integration and Testing** (1 week) - Comprehensive validation

## 💡 **Expected Combined Results**

**Conservative Estimates:**
- **Individual Optimizations**: 10-60% improvement each  
- **Combined Effect**: 80-150% overall improvement
- **Memory Usage**: 40-60% reduction
- **Startup Performance**: Maintained (due to lazy loading)

**Best Case Scenario:**
- **Performance**: 200-300% improvement on typical MiniScript programs
- **Memory**: 50-70% reduction  
- **Cache Efficiency**: Significant improvement due to smaller Values and better data locality

This plan builds on the excellent JIT foundation you've already created and focuses on the core interpreter optimizations that will benefit all MiniScript programs, not just JIT-eligible code paths.