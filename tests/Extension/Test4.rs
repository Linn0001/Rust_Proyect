fn main() {
    let mut sum: i64;
    let mut i: i64;
    sum = 0;

    for i in 1..11
    {
        sum = sum + (i * i);
    }

    println!("{}", sum);
}
