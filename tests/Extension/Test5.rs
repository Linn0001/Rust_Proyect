fn main() {
    let mut n: i64;
    let mut result: i64;

    n = 5;
    result = 1;

    while n > 0
    {
        result = result * n;
        n = n - 1;
    }

    println!("{}", result);
}
