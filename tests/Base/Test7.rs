fn test(a: i32) -> bool {
    return(a + 1);
}

fn main() {
    let mut b: i32 = 1;
    let x: i32 = test(1);
    println!("{}", x);
}