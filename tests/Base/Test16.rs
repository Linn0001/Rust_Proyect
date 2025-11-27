fn main() {
    let a: i16 = 10;
    let b: i64 = 100;

    let x: i64 = (a > 5) ? a : b;
    let y: i64 = (a > 50) ? a : b;

    println!("{}", x);
    println!("{}", y);
}
