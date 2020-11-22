use std::env;
use std::process::exit;
use rand::distributions::{Distribution, Uniform};
use chrono::{Datelike, Timelike, Utc};
use rand::Rng;
use std::fs;
use std::io::Write;

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
    for c in &circuit {
        print!("{} ({}) => ",c.0, c.1);
        for x in &c.2 {
            print!("{} ",x);
        }
        println!();
    }

    let now = Utc::now();

    let mut identifiers:Vec<char> = Vec::new();
    for i in 33..127 { identifiers.push(char::from(i)) }

    {
        let mut vcdout = fs::OpenOptions::new()
        .read(false)
        .write(true)
        .create(true)
        .open("out.vcd")
        .expect("Unable to open file");
        
        vcdout.set_len(0).expect("Error manipulating file");

        writeln!(&mut vcdout, "$date {}-{:02}-{:02} {:02}:{:02}:{:02} $end", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second()).expect("Error writing line");
        writeln!(&mut vcdout, "$version 1.0.0 $end").expect("Error writing line");
        writeln!(&mut vcdout, "$comment combinationalcl $end").expect("Error writing line");
        writeln!(&mut vcdout, "$timescale 1ns $end").expect("Error writing line");
        writeln!(&mut vcdout, "$scope module logic $end").expect("Error writing line");
        for i in 0..ic {
            writeln!(&mut vcdout, "$var wire 1 {} {} $end",helper::genid(&identifiers, i as usize),circuit[i as usize].0).expect("Error writing line");
        }
        writeln!(&mut vcdout, "$upscope $end").expect("Error writing line");
        writeln!(&mut vcdout, "$enddefinitions $end").expect("Error writing line");
        writeln!(&mut vcdout, "$dumpvars").expect("Error writing line");
        for i in 0..ic {
            writeln!(&mut vcdout, "{}{}",0,helper::genid(&identifiers, i as usize)).expect("Error writing line");
        }
        writeln!(&mut vcdout, "$end").expect("Error writing line");
        for t in 0..100 {
            writeln!(&mut vcdout, "#{}",t).expect("Error writing line");
            for i in 0..ic {
                writeln!(&mut vcdout, "{}{}",rng.gen_range(0,2),helper::genid(&identifiers, i as usize)).expect("Error writing line");
            }
        }
    }

    {
        let mut kernelout = fs::OpenOptions::new()
        .read(false)
        .write(true)
        .create(true)
        .open("out.cl")
        .expect("Unable to open file");

        kernelout.set_len(0).expect("Error manipulating file");

        writeln!(&mut kernelout, "__kernel void combinational(int ic, __global const char* inputs, int oc, __global char* outputs) {{").expect("Error writing line");
        writeln!(&mut kernelout,"int time = get_global_id(0);").expect("Error writing line");
        //writeln!(&mut kernelout,"int id = get_global_id(1);").expect("Error writing line");
        for i in 0..ic { //inputs
            writeln!(&mut kernelout,"char {} = inputs[time*ic+{}];", &circuit[i as usize].0, i).expect("Error writing line");
        }
        for i in ic..ic+nc { //inputs
            let mut outstr = "char ".to_string()+&circuit[i as usize].0.to_string()+&" = ".to_string();
            let mut second=false;
            for dp in &circuit[i as usize].2 {
                match &circuit[i as usize].1 {
                    '!' => {
                        outstr+=&("!".to_string()+&circuit[*dp as usize].0);
                    },
                    '.' => {
                        if second {
                            outstr+=&"&".to_string();
                        }
                        outstr+=&circuit[*dp as usize].0;
                    },
                    '+' => {
                        if second {
                            outstr+=&"|".to_string();
                        }
                        outstr+=&circuit[*dp as usize].0;
                    },
                    '^' => {
                        if second {
                            outstr+=&"^".to_string();
                        }
                        outstr+=&circuit[*dp as usize].0;
                    },
                    _ => {}
                }
                second=true;
            }
            outstr+=";";
            writeln!(&mut kernelout,"{}",outstr).expect("Error writing line");
        }
        for i in ic+nc..ic+nc+oc { //inputs
            writeln!(&mut kernelout,"outputs[time*oc+{}]={};", i-(ic+nc), &circuit[circuit[i as usize].2[0] as usize].0).expect("Error writing line");
        }
        writeln!(&mut kernelout,"}}").expect("Error writing line");
    }
}
