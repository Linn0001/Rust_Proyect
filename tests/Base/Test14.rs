fn main() {
    let a: i32 = 10;
    let b: i64 = 50;
    let c: u32 = 20;
    let d: f64 = 100.0;
    let e: f32 = 1.5;

    let r1: i64 = (a > 20) ? a : b;

    let r2: u64 = (c < 10) ? (c + 100) : 220;

    let r3: f64 = (e > 0.0) ? e : d;

    println!("{}", r1);
    println!("{}", r2);
    println!("{}", r3);
}
