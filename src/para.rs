#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]

mod ptg_header;

use core::ffi::c_void;
use std::mem::transmute;
use std::fs::File;
use std::io::BufReader;
use std::io::Read;
use std::env;
use std::process::exit;

use crate::ptg_header::*;



enum TokenType
{
    TokenPlus,
    TokenMinus,
    TokenTimes,
    TokenDivide,
    TokenEquals,
    TokenOpen,
    TokenClose,
    TokenNumber,
    TokenId,
    TokenEnd,
}

fn i64_to_TokenType(token_type: i64) -> Result<TokenType, &'static str>
{
    match token_type
    {
        0 => Ok(TokenType::TokenPlus),
        1 => Ok(TokenType::TokenMinus),
        2 => Ok(TokenType::TokenTimes),
        3 => Ok(TokenType::TokenDivide),
        4 => Ok(TokenType::TokenEquals),
        5 => Ok(TokenType::TokenOpen),
        6 => Ok(TokenType::TokenClose),
        7 => Ok(TokenType::TokenNumber),
        8 => Ok(TokenType::TokenId),
        9 => Ok(TokenType::TokenEnd),
        _ => Err("Invalid token type"),
    }
}


fn eval_tree(expr: &Expr)
{
    let mut expr_stack: [*const Expr; 1024] = [0 as *const Expr; 1024];
    
    match i64_to_TokenType(expr.token.token_type).unwrap() 
    {
        TokenType::TokenPlus => {

        },
        TokenType::TokenMinus => {

        },
        TokenType::TokenTimes => {

        },
        TokenType::TokenDivide => {

        },
        TokenType::TokenEquals => {

        },
        TokenType::TokenOpen => {

        },
        TokenType::TokenClose => {

        },
        TokenType::TokenNumber => {

        },
        TokenType::TokenId => {

        },
        TokenType::TokenEnd => {

        }, 
    }
}

fn main()
{
    load_ptg();
    let args: Vec<String> = env::args().collect();
    let mut token_list: Vec<ParseToken> = Vec::new();

    if args.len() < 2
    {
        eprintln!("ERROR: no command provided");
        exit(1);
    }
    
    let arg = args[1].as_bytes();
    {
        let mut i: usize = 0;
        while i < arg.len()
        {
            if arg[i].is_ascii_alphabetic()
            {
                let mut count = 1;
                while (i + count) != arg.len() && arg[i + count].is_ascii_alphanumeric()
                {
                    count += 1;
                }
                token_list.push(ParseToken {token_type: TokenType::TokenId as i64, data: &(arg[i] as i8), length: count as u32});
                i += count as usize - 1;
            }
            else if arg[i].is_ascii_alphanumeric()
            {
                let mut count = 1;
                while (i + count) != arg.len() && arg[i + count].is_ascii_alphanumeric()
                {
                    count += 1;
                }
                token_list.push(ParseToken {token_type: TokenType::TokenNumber as i64, data: &(arg[i] as i8), length: count as u32});
                i += count as usize - 1;
            }
            else
            {
                match arg[i]
                {
                    b'+' => token_list.push(ParseToken {token_type: TokenType::TokenPlus as i64, data: &(arg[i] as i8), length: 1}),
                    b'-' => token_list.push(ParseToken {token_type: TokenType::TokenMinus as i64, data: &(arg[i] as i8), length: 1}),
                    b'*' => token_list.push(ParseToken {token_type: TokenType::TokenTimes as i64, data: &(arg[i] as i8), length: 1}),
                    b'/' => token_list.push(ParseToken {token_type: TokenType::TokenDivide as i64, data: &(arg[i] as i8), length: 1}),
                    b'=' => token_list.push(ParseToken {token_type: TokenType::TokenEquals as i64, data: &(arg[i] as i8), length: 1}),
                    b'(' => token_list.push(ParseToken {token_type: TokenType::TokenOpen as i64, data: &(arg[i] as i8), length: 1}),
                    b')' => token_list.push(ParseToken {token_type: TokenType::TokenClose as i64, data: &(arg[i] as i8), length: 1}),
                    _ => {eprintln!("ERROR: Unknown value {}", arg[i]); exit(1);},
                }
            }
            i += 1;
        }
        token_list.push(ParseToken {token_type: TokenType::TokenEnd as i64, data: &(arg[0] as i8), length: 0});
    }
    let token_count = token_list.len() as u32;


    let mut bnf_src: Vec<u8>;
    {
        let file = File::open("./src/bnf.txt").unwrap();
        let mut buf_reader = BufReader::new(file);
        let mut bnf: String = String::new();
        buf_reader.read_to_string(&mut bnf).unwrap();
        bnf_src = bnf.into_bytes();
    }
    bnf_src.push(0);

    
    let mut table: [u8; 10000] = [0; 10000];
    let table_size: u32 = unsafe {write_parse_table_from_bnf(&mut table as *mut u8 as *mut c_void, 10000, bnf_src.as_ptr() as *const i8)};
    if table_size != 0
    {
        println!("failed to read table {}", table_size);
        return;
    }


    let mut expr: *mut Expr = 0 as *mut Expr;
    let mut msg: [u8; 1000] = [0; 1000];
    let success: bool = unsafe {parse_bin(token_list.as_mut_ptr(), token_count, table.as_mut_ptr(), 1, &mut expr, msg.as_mut_ptr() as *mut i8, 1000)};

    

    println!("success: {}, \n{}", success, std::str::from_utf8(&msg).unwrap());
    unsafe {graphviz_from_syntax_tree(b"./input.dot\0".as_ptr() as *const i8, expr)};
    unload_ptg();
}