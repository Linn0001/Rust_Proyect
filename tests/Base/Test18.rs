fn main() {
    let x: u32 = 5;
    let y: u64 = 40;
    let r1: u64 = (x < 10) ? (x + 10) : (y + 10);
    let r2: u64 = (x > 10) ? (x + 10) : (y + 10);

    println!("{}", r1);
    println!("{}", r2);
}
