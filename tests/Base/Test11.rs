fn main() {
    let a: i16 = 10;
    let b: i64 = 20;
    let c: u32 = 5;

    let d: i64 = a + b;
    let e: i64 = b + c;
    let f: u64 = c + 10;

    println!("{}", d);
    println!("{}", e);
    println!("{}", f);
}
