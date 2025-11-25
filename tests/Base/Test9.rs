fn process(a: i8, b: i32, c: i64) -> i64 {
    let result: i64 = c;
    result = result + b;
    return(result)
}

fn main() -> i32 {
    let x: i64 = process(5, 100, 1000);
    let y: i32 = 100;
    println!("{}", x + y);
    return(0)
}