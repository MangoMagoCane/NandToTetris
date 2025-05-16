#![allow(warnings)]
mod writers;

use std::env;
use std::error::Error;
use std::fs;
use std::fs::File;
use std::str::FromStr;
use std::path::Path;
use std::process;

use writers::{VMWriter, Command, Segment};

fn main() {
    let args: Vec<String> = env::args().collect();

    let config = Config::build(&args).unwrap_or_else(|err| {
        eprintln!("{err}");
        process::exit(1);
    });

    if let Err(e) = run(config) {
        eprintln!("{e}");
        process::exit(1);
    }

    process::exit(0);
}

fn run(config: Config) -> Result<(), Box<dyn Error>> {
    let mut writer = VMWriter::new();
    let buffer = File::create("foo.asm");
    writer.output = Box::new(buffer.unwrap());

    if config.path.is_file() {
        writer.start_program()?;
        handle_file(&config.path, &mut writer)?;
        writer.end_program()?;
    } else if config.path.is_dir() {
        handle_path(&config.path, &mut writer)?;
    } else {
    }
    Ok(())
}

fn handle_file(path : &Path, writer: &mut VMWriter) -> Result<(), Box<dyn Error>> {
    let contents = fs::read_to_string(path)?;
    writer.set_file_name(path);
    for (i, tokens) in contents.lines()
                        .map(|s| s.split("//").next().unwrap().trim())
                        .filter(|s| !s.starts_with("//") && s.len() > 0)
                        .map(|s| s.splitn(4, char::is_whitespace).collect::<Vec<_>>())
                        .enumerate() {
        // println!("{:?}", tokens);

        match tokens.len() {
            1 => {
                let command = Command::from_str(tokens[0])?;
                match command {
                    Command::Return => writer.ret()?,
                    _ => writer.arith_log(command)?,
                }
            }
            2 => {
                let command = Command::from_str(tokens[0])?;
                writer.branching(command, tokens[1]);
            },
            3 => {
                let command = Command::from_str(tokens[0])?;
                let segment = Segment::from_str(tokens[1])?;
                let Ok(index) = tokens[2].parse::<u16>() else {
                    return Err(format!("[line: {i}] Unparseable index.").into());
                };
                match command {
                    Command::Function => writer.function(segment, index)?,
                    Command::Call => writer.call(segment, index)?,
                    _ => writer.push_pop(command, segment, index)?,
                }
            },
            _ => {
                return Err(format!("[line: {i}] Invalid instruction.").into());
            }
        }
    }
    Ok(())
}

fn handle_path(_path: &Path, _writer: &mut VMWriter) -> Result<(), Box<dyn Error>> {
    Ok(())
}

struct Config<'a> {
    path: &'a Path,
}

impl <'a> Config<'a> {
    fn build(args: &[String]) -> Result<Config, &'static str> {
        if args.len() < 2 {
            return Err("usage: ./mips_vmtranslator [file.vm, dir/]");
        }

        let path = Path::new(&args[1]);
        Ok(Config { path })
    }
}
