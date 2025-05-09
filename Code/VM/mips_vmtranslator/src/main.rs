use std::env;
use std::fs;
use std::process;
use std::error::Error;

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
}

fn run(config: Config) -> Result<(), Box<dyn Error>> {
    match config {
        Config::File(file_path) => handle_file(&file_path),
        Config::Dir(dir_path) => handle_path(&dir_path),
    }?;
    Ok(())
}

fn handle_file(file_path: &str) -> Result<(), Box<dyn Error>> {
    let contents = fs::read_to_string(file_path)?;
    for line in contents.lines() {
        let buf: &str = line;
        println!("{buf}");
    }
    Ok(())
}

fn handle_path(dir_path: &str) -> Result<(), Box<dyn Error>> {
    Ok(())
}

enum Config {
    File(String),
    Dir(String),
}

impl Config {
    fn build(args: &[String]) -> Result<Config, &'static str> {
        if args.len() < 2 {
            return Err("usage: ./mips_vmtranslator [file.vm, dir]");
        }

        let path = args[1].clone();
        if path.chars().last().unwrap() == '/' {
            Ok(Config::Dir(path))
        } else {
            Ok(Config::File(path))
        }
    }
}
