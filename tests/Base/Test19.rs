fn main() {
    let a: f64 = 1.5;
    let b: f64 = 3.0;

    let mayor: f64 = (a > b) ? a : b;
    let menor: f64 = (a < b) ? a : b;

    let eq1: bool = a == b;
    let eq2: bool = menor == 1.5;

    println!("{}", mayor);
    println!("{}", menor);
    println!("{}", eq1);
    println!("{}", eq2);
}