fn suma_rango(inicio: i64, fin: i64) -> i64 {
    let mut total: i64;
    let mut i: i64;

    total = 0;
    i = inicio;

    while i <= fin
    {
        total = total + i;
        i = i + 1;
    }

    return(total);
}

fn main() {
    let mut r: i64;

    r = suma_rango(1, 10);
    println!("{}", r);
}
