fn test_floats() -> f32 {
    let a: f32 = 3.14;
    let b: f64 = 2.718281828;
    let c: f64 = 7.13;

    println!("{}", a);
    println!("{}", c);

    return(a)
}

fn main() {
    let x: f32 = test_floats();
    println!("{}", x);
}