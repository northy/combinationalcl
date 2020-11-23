use rand::Rng;

macro_rules! parseinput {
    ($x:expr) => {
        match $x.parse() {
            Ok(n) => {
                n
            },
            Err(_) => {
                eprintln!("error: argument {} not an integer", $x);
                return Ok(());
            },
        };
    };
}

pub fn genid(vec:&Vec<char>,mut id:usize)->String {
    let mut encoded:String = String::new();

    let base = vec.len();

    while id/base>0 {
        encoded = vec[id%base].to_string() + &encoded.to_string();
        id = id/base;
    }
    encoded = vec[id%base].to_string() + &encoded.to_string();

    encoded
}

pub fn choosecuts(id:i64,port:char)->Vec<i64> {
    let mut rng = rand::thread_rng();
    
    match port {
        '.' | '+' | '^' => vec![rng.gen_range(0,id),rng.gen_range(0,id)],
        '!' => vec![rng.gen_range(0,id)],
        _ => vec![]
    }
}