use std::env;
use std::process::exit;
use rand::distributions::{Distribution, Uniform};
use chrono::{Datelike, Timelike, Utc};

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
    let nc:i64 = parseinput!(args[1]);
    let ic:i64 = parseinput!(args[2]);
    let oc:i64 = parseinput!(args[3]);

    let mut circuit : Vec<(String,char,Vec<i64>)> = Vec::new();
    let ports:Vec<char> = vec!['+','.','!','^'];

    let sz:i64 = nc+ic+oc;
    let die = Uniform::from(0..ports.len());
    let mut rng = rand::thread_rng();
    for i in 0..sz {
        if i<ic { circuit.push(("i".to_string()+&i.to_string(),'i',vec![])); }
        else if sz-i<=oc { circuit.push(("o".to_string()+&(oc-(sz-i)).to_string(),'o',helper::choosecuts(i,'o'))); }
        else {
            let port:char = ports[die.sample(&mut rng)];
            circuit.push(("p".to_string()+&(i-ic).to_string(),port,helper::choosecuts(i,port)));
        }
    }
    /*for c in circuit {
        print!("{} ({}) => ",c.0, c.1);
        for x in c.2 {
            print!("{} ",x);
        }
        println!();
    }*/

    let now = Utc::now();

    let mut identifiers:Vec<char> = Vec::new();
    for i in 33..127 { identifiers.push(char::from(i)) }

    println!("$date {}-{:02}-{:02} {:02}:{:02}:{:02} $end", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    println!("$version 1.0.0 $end");
    println!("$comment combinationalcl $end");
    println!("$timescale 1ns $end");
    println!("$scope module logic $end");
    for i in 0..(&mut circuit).len() {
        println!("$var wire 1 {} {} $end",helper::genid(&identifiers, i),circuit[i].0);
    }
    println!("$upscope $end");
    println!("$enddefinitions $end");
}
