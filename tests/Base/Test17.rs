fn main() {
    let a: i32 = 3;
    let b: i32 = 7;

    let bigger_is_b: bool = (a > b) ? true : false;
    let equal: bool = (a == b) ? true : false;

    println!("{}", bigger_is_b);
    println!("{}", equal);
}
