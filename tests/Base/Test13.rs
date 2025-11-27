fn max(a: i32, b: i32) -> i32 {
    return (a > b) ? a : b;
}

fn main() {
    let x: i32 = 15;
    let y: i32 = 9;
    let m: i32 = max(x, y);

    println!("{}", m);
}
