fn main() {
    let a: i32 = 10;
    let b: i32 = 20;

    let max: i32 = (a > b) ? a : b;
    let min: i32 = (a > b) ? b : a;

    println!("{}", max);
    println!("{}", min);
}
