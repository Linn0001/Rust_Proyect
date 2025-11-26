use std::ops::Add;

struct Punto { x: i32, y: i32 }

impl Add for Punto {
    type Output = Punto;
    fn add(self, other: Punto) -> Punto {
        Punto { x: self.x + other.x, y: self.y + other.y }
    }
}

fn main() {
    let p1 = Punto { x: 1, y: 2 };
    let p2 = Punto { x: 3, y: 4 };
    let p3 = p1 + p2;
    println!("({}, {})", p3.x, p3.y);
}