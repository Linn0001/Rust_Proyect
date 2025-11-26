fn operator +(a: i32, b: i32) -> i32 {
    let base: i32 = a - (0 - b);
    return(base)
}

fn main() {
    let x: i32 = 5;
    let y: i32 = 8;
    let z: i32 = x + y;
    let native: i32 = x * y;
    println!("{}", z);
    println!("{}", native);
}