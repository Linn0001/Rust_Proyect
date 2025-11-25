fn test_sizes() -> i32 {
    let a: i8 = 10;
    let b: i16 = 1000;
    let c: i32 = 100000;
    let d: i64 = 999999;

    println!("{}", a);
    println!("{}", b);
    println!("{}", c);
    println!("{}", d);

    return(0)
}

fn main() {
    let x: i32 = test_sizes();
    println!("{}", x);
}