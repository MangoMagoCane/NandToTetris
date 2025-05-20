use std::collections::HashMap;
use std::error::Error;
use std::io;
use std::path::Path;
use std::str::FromStr;

use indoc::{formatdoc};

use tokenize_enum_derive::TokenizeEnum;
use Command::*;
use Segment::*;

pub struct VMWriter {
    pub output: Box<dyn io::Write>,
    file_name: String,
    func_name: String,
    pub is_dir: bool,
    log_jmp_i: u32,
    call_i: u32,
}

#[derive(Debug, TokenizeEnum)]
pub enum Command {
    Push, Pop,
    Add, Sub, Neg,
    Eq, Gt, Lt,
    And, Or, Not,
    Function, Call, Return,
    Label, Goto, If_goto
}

#[derive(Debug, TokenizeEnum)]
pub enum Segment {
    Local, Argument, This, That,
    Temp, Static, Pointer, Constant
}

impl VMWriter {
    pub fn new() -> VMWriter {
        VMWriter {
            output: Box::new(io::stdout()),
            file_name: String::from(""),
            func_name: String::from("_$global$_"),
            is_dir: false,
            log_jmp_i: 0,
            call_i: 0,
        }
    }

    pub fn set_file_name(&mut self, path: &Path) -> Result<(), Box<dyn Error>> {
        self.file_name = String::from(path.file_stem().unwrap().to_str().unwrap());
        self.write(formatdoc! {"
            #   --- file {0} ---
        ", self.file_name})
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
            data_offset:	.word	0:32768

            .text
            	li	$t0, 0xffff
            	sw	$t0, ($sp)
            	subi	$sp, $sp, 4
        "})?;

        if self.is_dir {
            self.write(formatdoc! {"
            # ----- bootstrapping call to Sys.init -----
            	j	Sys.init
            "})?;
        }

        Ok(())
    }

    pub fn end_program(&mut self) -> Result<(), Box<dyn Error>> {
        self.os()?;
        self.write(formatdoc! {"
            return_jmp:
            	move	$t0, $s0 #return_jmp
            	lw	$t1, 20($t0)
            	lw	$t2, 4($sp)
            	sw	$t2, ($s1)
            	subi	$sp, $s1, 4
            	lw	$s0, 16($t0)
            	lw	$s1, 12($t0)
            	lw	$s2, 8($t0)
            	lw	$s3, 4($t0)
            	jr	$t1
            call_jmp:
            	subi	$sp, $sp, 20 # call_jmp
            	sw	$ra, 20($sp)
            	sw	$s0, 16($sp)
            	sw	$s1, 12($sp)
            	sw	$s2, 8($sp)
            	sw	$s3, 4($sp)
            	addi	$t1, $t1, 20
            	add	$s1, $sp, $t1
            	move	$s0, $sp
            	jr	$t0
        "})
            // # ----- end_of_program -----
            // 	lw	$a0, 4($sp)
            // #	li	$v0, 1
            // #	syscall
            // 	li	$v0, 10
            // 	syscall
    }

    pub fn arith_log(&mut self, command: Command) -> Result<(), Box<dyn Error>> {
        let asm = match command {
            Add | Sub | And | Or  => formatdoc! {"
                # ----- binary: {command} -----
                	lw	$t0, 8($sp)
                	lw	$t1, 4($sp)
                	{command}	$t0, $t0, $t1
                	addi	$sp, $sp, 4
                	sw	$t0, 4($sp)
            "},
            Neg | Not => formatdoc! {"
                # ----- unary: {command} -----
                	lw	$t0, 4($sp)
                	{command}	$t0, $t0
                	sw	$t0, 4($sp)
            "},
            Eq | Gt | Lt => formatdoc! {"
                # ----- comp: {command} -----
                	lw	$t0, 8($sp)
                	lw	$t1, 4($sp)
                	li	$t3, 0
                	b{command}	$t0, $t1, {0}${1}
                	not	$t3, $t3
                	{0}${1}:
                	not	$t3, $t3
                	addi	$sp, $sp, 4
                	sw	$t3, 4($sp)
            ", self.file_name, {
                self.log_jmp_i += 1;
                self.log_jmp_i
            }},
            _ => return Err("".into()),
        };
        self.write(asm)
    }

    pub fn branching(&mut self, command: Command, label: &str) -> Result<(), Box<dyn Error>> {
        let jmp_label = format!("{}${}", self.func_name, label);

        let asm = match command {
            Label => formatdoc! {"
                {jmp_label}:
            "},
            Goto => formatdoc! {"
                \tj	{jmp_label}
            "},
            If_goto => formatdoc! {"
                \taddi	$sp, $sp, 4
                	lw	$t0, ($sp)
                	bnez	$t0, {jmp_label}
            "},
            _ => return Err("".into()),
        };

        self.write(formatdoc! {"
            # ----- {command} {label} ------
            {asm}"
        })
    }

    pub fn function(&mut self, func_name: &str, n_vars: u32) -> Result<(), Box<dyn Error>>{
        self.func_name = String::from(func_name);

        self.write(formatdoc! {"
            # ----- function {func_name} {n_vars} ------
            {func_name}:
        "})?;

        if n_vars > 0 {
            self.write(formatdoc! {"
                \tsubi	$sp, $sp, {0}
            ", n_vars * 4})?;
        }

        for i in (1..n_vars+1).rev() {
            self.write(formatdoc! {"
                \tsw	$zero, {0}($sp)
            ", i * 4})?;
        }

        Ok(())
    }

    pub fn call(&mut self, func_name: &str, n_args: u32) -> Result<(), Box<dyn Error>> {
        let asm = match func_name {
            "Math.multiply" => formatdoc! {"
                # ----- binary call: {func_name} -----
                	lw	$t0, 8($sp)
                	lw	$t1, 4($sp)
                	mul	$t0, $t0, $t1
                	addi	$sp, $sp, 4
                	sw	$t0, 4($sp)
            "},
            "Math.divide" => formatdoc! {"
                # ----- binary call: {func_name} -----
                	lw	$t0, 8($sp)
                	lw	$t1, 4($sp)
                	div	$t0, $t0, $t1
                	addi	$sp, $sp, 4
                	sw	$t0, 4($sp)
            "},
            _ => formatdoc! {"
                # ----- call {func_name} {n_args} ------
                	la	$t0, {func_name}
                	li	$t1, {0}
                	jal	call_jmp
            ", n_args * 4},
        };

        self.write(asm)
    }

    pub fn ret(&mut self) -> Result<(), Box<dyn Error>> {
        self.write(formatdoc! {"
            # ----- return ------
            	j	return_jmp
        "})
    }

    pub fn push_pop(&mut self, command: Command, segment: Segment, index: u32) -> Result<(), Box<dyn Error>> {
        let s_reg = match segment {
            Local => "$s0",
            Argument => "$s1",
            This => "$s2",
            That => "$s3",
            _ => "",
        };

        let word_index = index * 4;
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
                        \tsub	$t0, {s_reg}, {word_index}
                        	lw	$t0, ($t0)
                        	sw	$t0, ($sp)
                        	subi	$sp, $sp, 4
                    "},
                    This | That => formatdoc! {"
                        \tsll	$t1, {s_reg}, 2
                        	subi	$t0, $t1, {word_index}
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
                        \tsub	$t0, {s_reg}, {word_index}
                        	addi	$sp, $sp, 4
                        	lw	$t1, ($sp)
                        	sw	$t1, ($t0)
                    "},
                    This | That => formatdoc! {"
                        \tsll	$t1, {s_reg}, 2
                        	addi	$t0, $t1, {word_index}
                        	addi	$sp, $sp, 4
                        	lw	$t2, ($sp)
                        	sw	$t2, data_offset($t0)
                    "},
                    Static | Temp => formatdoc! {"
                        \tli	$t0, {word_index}
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
            {command_asm}"
        })
    }

    pub fn os(&mut self) -> Result<(), Box<dyn Error>> {
        self.write(formatdoc! {"
        # ----- os -----
        #   --- Sys ---
        Sys.init:
        	move	$s0, $sp
        	la	$t0, Main.main
        	li	$t1, 0
        	jal	call_jmp
        	li	$v0, 10
        	syscall
        "})?;

        self.write(formatdoc! {"
        #   --- Output ---
        Output.printInt:
        	lw	$a0, ($s1)
        	li	$v0, 1
        	syscall
        	li	$a0, 10	# newline
        	li	$v0, 11	#print byte
        	syscall
        	j	return_jmp
        "})
    }
}
