use std::env;
use std::process::exit;
use rand::distributions::{Distribution, Uniform};
use chrono::{Datelike, Timelike, Utc};
use rand::Rng;
use std::fs;
use std::io::Write;
use std::io;

#[macro_use]
mod helper;

fn main()->io::Result<()> {
    let args: Vec<String> = env::args().collect();
    if args.len()<5 {
        println!("Example input: ./gen n i o k
                n - port count
                i - input count
                o - output count
                k - events count");
        exit(1);
    }
    let nc:i64 = parseinput!(args[1]);
    let ic:i64 = parseinput!(args[2]);
    let oc:i64 = parseinput!(args[3]);
    let k:i64 = parseinput!(args[4]);

    let mut circuit : Vec<(String,char,Vec<i64>)> = Vec::new();
    let ports:Vec<char> = vec!['+','.','!','^'];

    let sz:i64 = nc+ic+oc;
    let die = Uniform::from(0..ports.len());
    let mut rng = rand::thread_rng();
    for i in 0..sz-oc {
        if i<ic { circuit.push(("i".to_string()+&i.to_string(),'i',vec![])); }
        //format!
        else {
            let port:char = ports[die.sample(&mut rng)];
            circuit.push(("p".to_string()+&(i-ic).to_string(),port,helper::choosecuts(i,port)));
        }
    }
    for i in sz-oc..sz { //outputs
        circuit.push(("o".to_string()+&(oc-(sz-i)).to_string(),'o',vec![rng.gen_range(0,sz-oc)]));
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

    let mut ids:Vec<String> = Vec::new();

    for i in 0..ic+nc+oc {
        ids.push(helper::genid(&identifiers, i as usize));
    }

    {
        let vcdout = &mut fs::OpenOptions::new()
        .read(false)
        .write(true)
        .create(true)
        .open("out.vcd")
        .expect("Unable to open file");

        vcdout.set_len(0).expect("Error manipulating file");

        writeln!(vcdout, "$date {}-{:02}-{:02} {:02}:{:02}:{:02} $end", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second()).expect("Error writing line");
        writeln!(vcdout, "$version 1.0.0 $end").expect("Error writing line");
        writeln!(vcdout, "$comment combinationalcl $end").expect("Error writing line");
        writeln!(vcdout, "$timescale 1ns $end").expect("Error writing line");
        writeln!(vcdout, "$scope module logic $end").expect("Error writing line");
        for i in 0..ic {
            writeln!(vcdout, "$var wire 1 {} {} $end",ids[i as usize],circuit[i as usize].0).expect("Error writing line");
        }
        writeln!(vcdout, "$upscope $end").expect("Error writing line");
        writeln!(vcdout, "$enddefinitions $end").expect("Error writing line");
        writeln!(vcdout, "$dumpvars").expect("Error writing line");
        for i in 0..ic {
            writeln!(vcdout, "{}{}",0,ids[i as usize]).expect("Error writing line");
        }
        writeln!(vcdout, "$end").expect("Error writing line");
        for t in 0..k {
            writeln!(vcdout, "#{}",t).expect("Error writing line");
            for i in 0..ic {
                writeln!(vcdout, "{}{}",rng.gen_range(0,2),ids[i as usize]).expect("Error writing line");
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

        writeln!(&mut kernelout, "__kernel void combinational(const int ic, __global const char* inputs, const int pc, __global char* ports, const int oc, __global char* outputs) {{").expect("Error writing line");
        writeln!(&mut kernelout,"int time = get_global_id(0);").expect("Error writing line");
        for i in 0..ic { //inputs
            writeln!(&mut kernelout,"const char {} = inputs[time*ic+{}];", &circuit[i as usize].0, i).expect("Error writing line");
        }
        for i in ic..ic+nc { //ports
            let mut outstr = format!("const char {} = ports[time*pc+{}] = ",circuit[i as usize].0,i-ic);
            let mut second=false;
            for dp in &circuit[i as usize].2 {
                match &circuit[i as usize].1 {
                    '!' => {
                        outstr.push_str("!");
                        outstr.push_str(&circuit[*dp as usize].0);
                    },
                    '.' => {
                        if second {
                            outstr.push_str("&");
                        }
                        outstr+=&circuit[*dp as usize].0;
                    },
                    '+' => {
                        if second {
                            outstr.push_str("|");
                        }
                        outstr+=&circuit[*dp as usize].0;
                    },
                    '^' => {
                        if second {
                            outstr.push_str("^");
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
        for i in ic+nc..ic+nc+oc { //outputs
            writeln!(&mut kernelout,"outputs[time*oc+{}]={};", i-(ic+nc), &circuit[circuit[i as usize].2[0] as usize].0).expect("Error writing line");
        }
        writeln!(&mut kernelout,"}}").expect("Error writing line");
    }

    Ok(())
}
