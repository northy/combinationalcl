use rand::Rng;

macro_rules! parseinputi32 {
    ($x:expr) => {
        match $x.parse() {
            Ok(n) => {
                n
            },
            Err(_) => {
                eprintln!("error: argument {} not an integer", $x);
                return;
            },
        };
    };
}

pub fn choosecuts(id:i64,port:char)->Vec<i64> {
    let mut rng = rand::thread_rng();
    
    match port {
        '.' | '+' | '^' => vec![rng.gen_range(0,id),rng.gen_range(0,id)],
        '!' | 'o' => vec![rng.gen_range(0,id)],
        _ => vec![]
    }
}