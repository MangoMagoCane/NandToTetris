use std::collections::HashMap;
use std::error::Error;
use std::io;
use std::path::Path;
use std::str::FromStr;

use indoc::{formatdoc};

use extend_enum_derive::ExtendEnum;
use Command::*;
use Segment::*;

pub struct VMWriter {
    pub output: Box<dyn io::Write>,
    file_name: String,
    func_name: String,
    is_dir: bool,
    log_jmp_i: u32,
    call_i: u32,
}

#[derive(Debug, ExtendEnum)]
pub enum Command {
    Push, Pop,
    Add, Sub, Neg,
    Eq, Gt, Lt,
    And, Or, Not,
    Function, Call, Return,
    Label, Goto, If_goto
}


#[derive(Debug, ExtendEnum)]
pub enum Segment {
    Local, Argument, This, That,
    Temp, Static, Pointer, Constant
}

impl VMWriter {
    pub fn new() -> VMWriter {
        VMWriter {
            output: Box::new(io::stdout()),
            file_name: String::from(""),
            func_name: String::from(""),
            is_dir: false,
            log_jmp_i: 0,
            call_i: 0,
        }
    }

    pub fn set_file_name(&mut self, path: &Path) {
        self.file_name = String::from(path.file_stem().expect("").to_str().expect(""));
    }

    fn write(&mut self, text: String) -> Result<(), Box<dyn Error>> {
        self.output.write(text.as_bytes())?;
        Ok(())
    }

    pub fn start_program(&mut self) -> Result<(), Box<dyn Error>> {
        self.write(formatdoc! {"
            .data
            temp:		.word	0:8
            static:		.word	0:240
            data_offset: .word 0

            .text
            	li	$t0, 0xffff
            	sw	$t0, ($sp)
            	subi	$sp, $sp, 4
        "})
    }

    pub fn end_program(&mut self) -> Result<(), Box<dyn Error>> {
        self.write(formatdoc! {"
            # ----- end_of_program -----
            	lw	$a0, 4($sp)
            	li	$v0, 1
            	syscall
            	li	$v0, 10
            	syscall
        "})?;
        Ok(())
    }

    pub fn arith_log(&mut self, command: Command) -> Result<(), Box<dyn Error>> {
        let asm = match command {
            Add | Sub | And | Or  => formatdoc! {"
                # ----- binary: {0} -----
                	lw	$t0, 8($sp)
                	lw	$t1, 4($sp)
                	{0}	$t0, $t0, $t1
                	addi	$sp, $sp, 4
                	sw	$t0, 4($sp)
                ", command},
            Neg | Not => formatdoc! {"
                # ----- unary: {0} -----
                	lw	$t0, 4($sp)
                	{0}	$t0, $t0
                	sw	$t0, 4($sp)
                ", command},
            Eq | Gt | Lt => formatdoc! {"
                # ----- comp: {0} -----
                	lw	$t0, 8($sp)
                	lw	$t1, 4($sp)
                	li	$t3, 0
                	b{0}	$t0, $t1, {1}${2}
                	not	$t3
                	{1}${2}:
                	not	$t3
                	addi	$sp, $sp, 4
                	sw	$t3, ($sp)
                ", command, self.file_name, {
                    self.log_jmp_i += 1;
                    self.log_jmp_i
                }},
            _ => return Err("".into()),
        };
        self.write(asm)
    }

    pub fn ret(&mut self) -> Result<(), Box<dyn Error>> {
        Ok(())
    }

    pub fn branching(&mut self, _command: Command, _label: &str) -> Result<(), Box<dyn Error>> {
        Ok(())
    }

    pub fn function(&mut self, _segment: Segment, _index: u16) -> Result<(), Box<dyn Error>>{
        Ok(())
    }

    pub fn call(&mut self, _segment: Segment, _index: u16) -> Result<(), Box<dyn Error>> {
        Ok(())
    }

    pub fn push_pop(&mut self, command: Command, segment: Segment, index: u16) -> Result<(), Box<dyn Error>> {
        let s_reg = match segment {
            Local => "$s0",
            Argument => "$s1",
            This => "$s2",
            That => "$s3",
            _ => "",
        };

        let mut pointer_reg = "";

        if matches!(segment, Static) && index > 240 {
            return Err("".into());
        } else if matches!(segment, Temp) && index > 7{
            return Err("".into());
        } else if matches!(segment, Pointer) {
            pointer_reg = match index {
                0 => "$s2",
                1 => "$s3",
                _ => return Err("".into()),
            }
        }

        let command_asm = match command {
            Push => {
                match segment {
                    Local | Argument => formatdoc! {"
                        \tli	$t0, {index}
                        	sll	$t0, $t0, 2
                        	add	$t0, $t0, {s_reg}
                        	lw	$t0, ($t0)
                        	sw	$t0, ($sp)
                        	subi	$sp, $sp, 4
                    "},
                    This | That => formatdoc! {"
                        \tli	$t0, {index}
                        	sll	$t0, $t0, 2
                        	sll	$t1, {s_reg}, 2
                        	add	$t0, $t0, $t1
                        	lw	$t0, data_offset($t0)
                        	sw	$t0, ($sp)
                        	subi	$sp, $sp, 4
                    "},
                    Static | Temp => formatdoc! {"
                        \tli	$t0, {index}
                        	sll	$t0, $t0, 2
                        	lw	$t0, {segment}($t0)
                        	sw	$t0, ($sp)
                        	subi	$sp, $sp, 4
                    "},
                    Pointer => formatdoc! {"
                        \tsw	{pointer_reg}, ($sp)
                        	subi	$sp, $sp, 4
                    "},
                    Constant => formatdoc! {"
                        \tli	$t0, {index}
                        	sw	$t0, ($sp)
                        	subi	$sp, $sp, 4
                    "},
                    _ => formatdoc! {""},
                }
            },
            Pop => {
                match segment {
                    Local | Argument => formatdoc! {"
                        \tli	$t0, {index}
                        	sll	$t0, $t0, 2
                        	add	$t0, $t0, {s_reg}
                        	addi	$sp, $sp, 4
                        	lw	$t1, ($sp)
                        	sw	$t1, ($t0)
                    "},
                    This | That => formatdoc! {"
                        \tli	$t0, {index}
                        	sll	$t0, $t0, 2
                          sll	$t1, {s_reg}, 2
                        	add	$t0, $t0, $t1
                        	addi	$sp, $sp, 4
                        	lw	$t2, ($sp)
                        	sw	$t2, data_offset($t0)
                    "},
                    Static | Temp => formatdoc! {"
                        \tli	$t0, {index}
                        	sll	$t0, $t0, 2
                        	addi	$sp, $sp, 4
                        	lw	$t1, ($sp)
                        	sw	$t1, {segment}($t0)
                    "},
                    Pointer => formatdoc! {"
                        \taddi	$sp, $sp, 4
                        	lw	{pointer_reg}, ($sp)
                    "},
                    _ => return Err("".into()),
                }
            },
            _ => return Err("".into()),
        };

        self.write(formatdoc! {"
            # ----- {command} {segment} {index} -----
            {command_asm}"})
    }
}
