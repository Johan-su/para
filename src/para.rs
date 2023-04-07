#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]

mod ptg_header;

use core::ffi::c_void;
use core::panic;
use std::fs::File;
use std::io::BufReader;
use std::io::Read;
use std::env;
use std::mem::transmute;
use std::process::exit;
use std::result;

use crate::ptg_header::*;



enum LR_Type
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
    ExpraltS,
    ExprFuncDecl,
    ExprVarDecl,
    ExprE,
    ExprFuncCall,
    ExprVar,
}

fn i64_to_TokenType(token_type: i64) -> Result<LR_Type, &'static str>
{
    dbg!(token_type);
    match token_type
    {
        0 => Ok(LR_Type::TokenPlus),
        1 => Ok(LR_Type::TokenMinus),
        2 => Ok(LR_Type::TokenTimes),
        3 => Ok(LR_Type::TokenDivide),
        4 => Ok(LR_Type::TokenEquals),
        5 => Ok(LR_Type::TokenOpen),
        6 => Ok(LR_Type::TokenClose),
        7 => Ok(LR_Type::TokenNumber),
        8 => Ok(LR_Type::TokenId),
        9 => Ok(LR_Type::TokenEnd),
        10 => Ok(LR_Type::ExpraltS),
        11 => Ok(LR_Type::ExprFuncDecl),
        12 => Ok(LR_Type::ExprVarDecl),
        13 => Ok(LR_Type::ExprE),
        14 => Ok(LR_Type::ExprFuncCall),
        15 => Ok(LR_Type::ExprVar),
        _ => Err("Invalid token type"),
    }
}

unsafe fn parse_number(data: *const i8, data_length: u32) -> i64
{
    let mut result: i64 = 0;

    for i in (0..data_length).rev()
    {
        let val = *(data.offset(i.try_into().unwrap())) as u8 as char; 
        match val
        {
            '0' => {result += 0 * i as i64}
            '1' => {result += 1 * i as i64}
            '2' => {result += 2 * i as i64}
            '3' => {result += 3 * i as i64}
            '4' => {result += 4 * i as i64}
            '5' => {result += 5 * i as i64}
            '6' => {result += 6 * i as i64}
            '7' => {result += 7 * i as i64}
            '8' => {result += 8 * i as i64}
            '9' => {result += 9 * i as i64}
            _ => {panic!("invalid number, got {}", val)}
        }

    }
    return result;
}


unsafe fn get_expr_in_array(expr: *const Expr, index: isize) -> *const Expr
{
    let exprs: *const *const Expr = transmute(expr.offset(1));
    return *exprs.offset(index);   
}


unsafe fn eval_tree(expr: *const Expr) -> i64
{
    dbg!(&*expr);
    match i64_to_TokenType((*expr).token.token_type).unwrap() 
    {
        LR_Type::TokenPlus => {panic!("unreachable")}
        LR_Type::TokenMinus => {panic!("unreachable")}
        LR_Type::TokenTimes => {panic!("unreachable")}
        LR_Type::TokenDivide => {panic!("unreachable")}
        LR_Type::TokenEquals => {
            assert_eq!((*expr).expr_count, 3);
            return eval_tree(get_expr_in_array(expr, 2));
        }
        LR_Type::TokenOpen => {todo!("not implemented")}
        LR_Type::TokenClose => {todo!("not implemented")}
        LR_Type::TokenNumber => {return parse_number((*expr).token.data, (*expr).token.length)}
        LR_Type::TokenId => {todo!("not implemented")}
        LR_Type::TokenEnd => {panic!("unreachable")}
        LR_Type::ExpraltS => {
            assert_eq!((*expr).expr_count, 1);
            return eval_tree(get_expr_in_array(expr, 0));
        }
        LR_Type::ExprFuncDecl => {todo!("not implemented")}
        LR_Type::ExprVarDecl => {todo!("not implemented")}
        LR_Type::ExprE => {
            match (*expr).expr_count
            {
                1 => {return eval_tree(get_expr_in_array(expr, 0))}
                2 => {todo!("not implemented")}
                3 => {match i64_to_TokenType((*get_expr_in_array(expr, 1)).token.token_type).unwrap() 
                {
                    LR_Type::TokenPlus => {
                        return eval_tree(get_expr_in_array(expr, 0)) + eval_tree(get_expr_in_array(expr, 2));
                    }
                    LR_Type::TokenMinus => {
                        return eval_tree(get_expr_in_array(expr, 0)) - eval_tree(get_expr_in_array(expr, 2));
                    }
                    LR_Type::TokenTimes => {
                        return eval_tree(get_expr_in_array(expr, 0)) * eval_tree(get_expr_in_array(expr, 2));
                    }
                    LR_Type::TokenDivide => {
                        return eval_tree(get_expr_in_array(expr, 0)) / eval_tree(get_expr_in_array(expr, 2));
                    }                 
                    _ => panic!("unreachable")
                }
}
                _ => panic!("unreachable")
            }
        }
        LR_Type::ExprFuncCall => {todo!("not implemented")}
        LR_Type::ExprVar => {todo!("not implemented")}
    
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
                token_list.push(ParseToken {token_type: LR_Type::TokenId as i64, data: &(arg[i] as i8), length: count as u32});
                i += count as usize - 1;
            }
            else if arg[i].is_ascii_alphanumeric()
            {
                let mut count = 1;
                while (i + count) != arg.len() && arg[i + count].is_ascii_alphanumeric()
                {
                    count += 1;
                }
                token_list.push(ParseToken {token_type: LR_Type::TokenNumber as i64, data: &(arg[i] as i8), length: count as u32});
                i += count as usize - 1;
            }
            else
            {
                match arg[i]
                {
                    b'+' => token_list.push(ParseToken {token_type: LR_Type::TokenPlus as i64, data: &(arg[i] as i8), length: 1}),
                    b'-' => token_list.push(ParseToken {token_type: LR_Type::TokenMinus as i64, data: &(arg[i] as i8), length: 1}),
                    b'*' => token_list.push(ParseToken {token_type: LR_Type::TokenTimes as i64, data: &(arg[i] as i8), length: 1}),
                    b'/' => token_list.push(ParseToken {token_type: LR_Type::TokenDivide as i64, data: &(arg[i] as i8), length: 1}),
                    b'=' => token_list.push(ParseToken {token_type: LR_Type::TokenEquals as i64, data: &(arg[i] as i8), length: 1}),
                    b'(' => token_list.push(ParseToken {token_type: LR_Type::TokenOpen as i64, data: &(arg[i] as i8), length: 1}),
                    b')' => token_list.push(ParseToken {token_type: LR_Type::TokenClose as i64, data: &(arg[i] as i8), length: 1}),
                    _ => {eprintln!("ERROR: Unknown value {}", arg[i]); exit(1);},
                }
            }
            i += 1;
        }
        token_list.push(ParseToken {token_type: LR_Type::TokenEnd as i64, data: &(arg[0] as i8), length: 0});
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
    let success: bool = unsafe {parse_bin(token_list.as_mut_ptr(), token_count, table.as_mut_ptr(), PRINT_EVERY_PARSE_STEP, &mut expr, msg.as_mut_ptr() as *mut i8, 1000)};

    

    println!("success: {}, \n{}", success, std::str::from_utf8(&msg).unwrap());
    unsafe {graphviz_from_syntax_tree(b"./input.dot\0".as_ptr() as *const i8, expr)};

    println!("evaled to {}", unsafe {eval_tree(expr)});

    unload_ptg();
}