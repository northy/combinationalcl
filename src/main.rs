use std::env;
use std::process::exit;
use rand::distributions::{Distribution, Uniform};

#[macro_use]
mod helper;

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len()<4 {
        println!("Example input: ./gen n i o
                n - port count
                i - input count
                o - output count");
        exit(1);
    }
    let nc:i64 = parseinputi32!(args[1]);
    let ic:i64 = parseinputi32!(args[2]);
    let oc:i64 = parseinputi32!(args[3]);

    let mut circuit : Vec<(char,Vec<i64>)> = Vec::new();
    let ports:Vec<char> = vec!['+','.','!','^'];

    let sz:i64 = nc+ic+oc;
    let die = Uniform::from(0..ports.len());
    let mut rng = rand::thread_rng();
    for i in 0..sz {
        if i<ic { circuit.push(('i',vec![])); }
        else if sz-i<=oc { circuit.push(('o',helper::choosecuts(i,'o'))); }
        else {
            let port:char = ports[die.sample(&mut rng)];
            circuit.push((port,helper::choosecuts(i,port)));
        }
    }
    for c in circuit {
        print!("{} => ",c.0);
        for x in c.1 {
            print!("{} ",x);
        }
        println!();
    }
   
}
