#![no_main]

use libfuzzer_sys::fuzz_target;
use asterix::{parse, ParseOptions};

fuzz_target!(|data: &[u8]| {
    // Skip empty input (already validated in the parser)
    if data.is_empty() {
        return;
    }

    // Skip oversized input (already validated, max 64KB)
    if data.len() > 65536 {
        return;
    }

    // Fuzz the parse function
    // We don't care if parsing succeeds or fails,
    // we just want to make sure it doesn't crash/panic
    let opts = ParseOptions::default();
    let _ = parse(data, opts);
});
