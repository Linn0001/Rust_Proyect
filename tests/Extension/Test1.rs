fn test_floats() -> f64 {
    let b: f64 = 2.718281828;
    let c: f64 = 7.13;

    println!("{}", b);
    println!("{}", c);

    return(b)
}

fn main() {
    let x: f64 = test_floats();
    println!("{}", x);
}