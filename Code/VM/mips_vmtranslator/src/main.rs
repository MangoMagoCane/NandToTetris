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

    if !config.print_to_stdout {
        let path = config.path;
        let buffer = File::create(path.join(path.file_stem().unwrap()).with_extension("asm")).unwrap();
        writer.output = Box::new(buffer);
    }

    if config.path.is_file() {
        handle_file(&config.path, &mut writer)?;
    } else if config.path.is_dir() {
        handle_dir(&config.path, &mut writer)?;
    } else {
        return Err("".into());
    }

    Ok(())
}


fn compile_file(path: &Path, writer: &mut VMWriter) -> Result<(), Box<dyn Error>> {
    let contents = fs::read_to_string(path)?;
    writer.set_file_name(path)?;
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
                writer.branching(command, tokens[1])?;
            },
            3 => {
                let command = Command::from_str(tokens[0])?;
                let Ok(index) = tokens[2].parse::<u32>() else {
                    return Err(format!("[line: {i}] Unparseable index.").into());
                };
                match command {
                    Command::Function => writer.function(tokens[1], index)?,
                    Command::Call => writer.call(tokens[1], index)?,
                    _ => writer.push_pop(command, Segment::from_str(tokens[1])?, index)?,
                }
            },
            _ => {
                return Err(format!("[line: {i}] Invalid instruction.").into());
            }
        }
    }

    Ok(())
}

fn handle_file(file_path: &Path, writer: &mut VMWriter) -> Result<(), Box<dyn Error>> {
    if !file_path.ends_with(".vm") {
        return Err("".into());
    }

    writer.start_program()?;
    compile_file(file_path, writer)?;
    writer.end_program()?;

    Ok(())
}

fn handle_dir(dir_path: &Path, writer: &mut VMWriter) -> Result<(), Box<dyn Error>> {
    writer.is_dir = true;

    writer.start_program()?;
    for entry in fs::read_dir(dir_path).unwrap() {
        let Ok(entry) = entry else { continue };
        let Ok(file_type) = entry.file_type() else { continue };
        if !file_type.is_file() { continue }

        let entry_path = entry.path();
        match entry_path.extension() {
            Some(ext) => if ext != "vm" { continue },
            None => continue,
        }

        compile_file(&entry_path, writer)?;
    }
    writer.end_program()?;

    Ok(())
}

#[derive(Debug)]
struct Config<'a> {
    path: &'a Path,
    print_to_stdout: bool
}

impl <'a> Config<'a> {
    fn build(args: &[String]) -> Result<Config, &'static str> {
        let argc = args.len();
        if argc < 2 {
            return Err("usage: ./mips_vmtranslator [file.vm, dir/]");
        }

        let path = Path::new(&args[1]);
        let mut print_to_stdout = false;

        for i in 3..argc {
            match args[i].as_str() {
                "-p" => print_to_stdout = true,
                _ => (),
            }
        }

        Ok(Config { path, print_to_stdout })
    }
}
