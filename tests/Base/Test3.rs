fn test_sizes() -> i32 {
    let b: i16 = 1000000000;
    let c: i32 = 1000000000;
    let d: i64 = 10000000000;

    println!("{}", b);
    println!("{}", c);
    println!("{}", d);

    return(c);
}

fn main() {
    let x: i32 = test_sizes();
    println!("{}",x);
}