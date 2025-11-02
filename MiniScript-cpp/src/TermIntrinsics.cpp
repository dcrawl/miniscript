#include "TermIntrinsics.h"
#include "TermIO.h"

#include "MiniScript/MiniscriptInterpreter.h"
#include "MiniScript/MiniscriptTypes.h"
#include "MiniScript/MiniscriptIntrinsics.h"

using namespace MiniScript;

// Helpers to get strings/numbers safely from arg lists.
static bool AsBool(Value v, bool def=false) {
    if (v.type == ValueType::Number) return v.DoubleValue() != 0.0;
    if (v.type == ValueType::String) return v.ToString().Length() > 0;
    return def;
}

static IntrinsicResult Intrinsic_term_raw(Context *context, IntrinsicResult args) {
    bool on       = context->GetVar("on").BoolValue();
    bool vtOut    = context->GetVar("vtOut").BoolValue();
    bool ok = TermIO::EnableRaw(on, vtOut);
    if (!ok) {
        return Value(("error: " + TermIO::lastError()).c_str());
    }
    return IntrinsicResult( on ? 1.0 : 0.0 );
}

static IntrinsicResult Intrinsic_term_read(Context *context, IntrinsicResult args) {
    int maxBytes   = context->GetVar("maxBytes").IntValue();
    int timeoutMs  = context->GetVar("timeoutMs").IntValue();
    if (maxBytes <= 0) return Value::null;

    std::string s;
    s.resize((size_t)maxBytes);
    int n = TermIO::Read(&s[0], maxBytes, timeoutMs);
    if (n < 0) return Value( Value(("error: " + TermIO::lastError()).c_str()) );
    s.resize((size_t)n);
    return IntrinsicResult( Value(s.c_str()) );
}

static IntrinsicResult Intrinsic_term_size(Context *context, IntrinsicResult args) {
    int rows=0, cols=0;
    if (!TermIO::GetSize(rows, cols)) {
        return Value( Value(("error: " + TermIO::lastError()).c_str()) );
    }
    // Return as a MiniScript list [rows, cols]
    ValueList li; li.Add( Value((double)rows) ); li.Add( Value((double)cols) );
    return IntrinsicResult(li);
}

void AddTermIntrinsics() {
    // We'll expose a small term map with functions raw/read/size.
    // term.raw(on=true, vtOut=true) -> 1 or 0 (and returns "error: ..." on failure)
    // term.read(maxBytes=1, timeoutMs=null) -> string ("" on timeout)
    // term.size() -> [rows, cols]

    Intrinsic* termRaw  = Intrinsic::Create("term_raw");
    termRaw->AddParam("on");
    termRaw->AddParam("vtOut", 1.0);
    termRaw->code = &Intrinsic_term_raw;

    Intrinsic* termRead = Intrinsic::Create("term_read");
    termRead->AddParam("maxBytes", 1.0);
    termRead->AddParam("timeoutMs", Value::null);
    termRead->code = &Intrinsic_term_read;

    Intrinsic* termSize = Intrinsic::Create("term_size");
    termSize->code = &Intrinsic_term_size;
}
