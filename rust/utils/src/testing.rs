#[macro_export]
macro_rules! expect_result {
    ($expected:expr, $actual:expr) => {{
        let expected = const { $expected };
        let start = ::std::time::Instant::now();
        let actual = $actual;
        let elapsed = start.elapsed();
        if expected != actual {
            panic!(
                "FAIL {} ms: expected {expected}, got {actual}",
                elapsed.as_millis()
            );
        }
        println!("  OK {} ms", elapsed.as_millis());
    }};
}

#[macro_export]
macro_rules! time {
    ($func:expr $(, $arg:expr)* $(,)?) => {{
        let start = ::std::time::Instant::now();
        let result = $func($($arg),*);
        let elapsed = start.elapsed();
        println!("  {} {} ms", stringify!($func), elapsed.as_millis());
        result
    }};
}
