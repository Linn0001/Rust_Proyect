fn test_floats() -> f64 {
    let a: f64 = 2.45;
    let b: f64 = 2.55;

    println!("{}", a);
    println!("{}", b);
    println!("{}", a + b);

    if a > b {
        return(a);
    }
    else {
        return(b);
    }
}

fn main() {
    let x: f64 = test_floats();
    println!("{}", x);
}