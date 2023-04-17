#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]

mod ptg_header;

use core::ffi::c_void;
use core::panic;
use std::collections::HashMap;
use std::fs::File;
use std::io;
use std::io::BufReader;
use std::io::Read;
use std::mem::transmute;
use std::slice::from_raw_parts;

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

unsafe fn parse_number(data: *const i8, data_length: u32) -> f64
{
    let mut result: f64 = 0.0;
    let mut num_pos: f64 = 1.0;
    for i in (0..data_length).rev()
    {
        let val = *(data.offset(i.try_into().unwrap())) as u8 as char; 
        match val
        {
            '0' => {/*result += 0.0 * num_pos*/}
            '1' => {result += 1.0 * num_pos}
            '2' => {result += 2.0 * num_pos}
            '3' => {result += 3.0 * num_pos}
            '4' => {result += 4.0 * num_pos}
            '5' => {result += 5.0 * num_pos}
            '6' => {result += 6.0 * num_pos}
            '7' => {result += 7.0 * num_pos}
            '8' => {result += 8.0 * num_pos}
            '9' => {result += 9.0 * num_pos}
            _ => {panic!("invalid number, got {}", val)}
        }
        num_pos *= 10.0;
    }
    return result;
}

unsafe fn print_i8(data: *const i8, data_length: u32)
{
    for i in 0..data_length
    {
        print!("{}", (*(data.offset(i as isize)) as u8 as char));
    }
    println!();
}



unsafe fn get_expr_in_array(expr: *const Expr, index: isize) -> *const Expr
{
    let exprs: *const *const Expr = transmute(expr.offset(1));
    return *exprs.offset(index);   
}


unsafe fn eval_tree(expr: *const Expr, map: &mut HashMap<&[i8], *const Expr>) -> f64
{
    match i64_to_TokenType((*expr).token.token_type).unwrap() 
    {
        LR_Type::TokenPlus => {panic!("unreachable")}
        LR_Type::TokenMinus => {panic!("unreachable")}
        LR_Type::TokenTimes => {panic!("unreachable")}
        LR_Type::TokenDivide => {panic!("unreachable")}
        LR_Type::TokenEquals => {todo!("not implemented")}
        LR_Type::TokenOpen => {todo!("not implemented")}
        LR_Type::TokenClose => {todo!("not implemented")}
        LR_Type::TokenNumber => {return parse_number((*expr).token.data, (*expr).token.length)}
        LR_Type::TokenId => {todo!("not implemented")}
        LR_Type::TokenEnd => {panic!("unreachable")}
        LR_Type::ExpraltS => {
            if (*expr).expr_count != 1
            {
                return 0.0;
            }
            return eval_tree(get_expr_in_array(expr, 0), map);
        }
        LR_Type::ExprFuncDecl => {todo!("not implemented")}
        LR_Type::ExprVarDecl => 
        {
            assert_eq!((*expr).expr_count, 3);

            let var_name_ptr: *const Expr = get_expr_in_array(expr, 2);
            let var_expr_ptr: *const Expr = get_expr_in_array(expr, 0);

            let slice: &[i8] = from_raw_parts((*var_name_ptr).token.data, (*var_name_ptr).token.length as usize);
            map.insert(slice, var_expr_ptr);
            return 0.0;
        }
        LR_Type::ExprE => {
            match (*expr).expr_count
            {
                1 => {return eval_tree(get_expr_in_array(expr, 0), map)}
                2 => 
                {
                    let token = i64_to_TokenType((*get_expr_in_array(expr, 1)).token.token_type).unwrap(); 
                    match token 
                    {
                        LR_Type::TokenPlus => {return eval_tree(get_expr_in_array(expr, 0), map)}
                        LR_Type::TokenMinus => {return -eval_tree(get_expr_in_array(expr, 0), map)}
                        _ => panic!("unexpected token")          
                    }
                }
                3 => 
                {
                    match i64_to_TokenType((*get_expr_in_array(expr, 1)).token.token_type).unwrap() 
                    {
                        LR_Type::TokenPlus =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map); 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map); 
                            return num1 + num2;
                        }
                        LR_Type::TokenMinus =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map); 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map); 
                            return num1 - num2;
                        }
                        LR_Type::TokenTimes =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map); 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map); 
                            return num1 * num2;
                        }
                        LR_Type::TokenDivide =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map); 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map); 
                            return num1 / num2;
                        }
                        LR_Type::ExprE =>
                        {
                            return eval_tree(get_expr_in_array(expr, 1), map);
                        }
                        _ => panic!("unreachable")
                    }
                }
                _ => panic!("unreachable")
            }
        }
        LR_Type::ExprFuncCall => {todo!("not implemented")}
        LR_Type::ExprVar => 
        {
            assert_eq!((*expr).expr_count, 1);

            let slice: &[i8] = from_raw_parts((*get_expr_in_array(expr, 0)).token.data, (*get_expr_in_array(expr, 0)).token.length as usize);            
            let val = map.get(slice);
            if val.is_none()
            {
                return 0.0
            }
            else 
            {
                return eval_tree(*val.unwrap(), map);    
            }
        }
    
    }
}




fn string_to_tokens<'a>(arg: &'a Vec<u8>, out: &'a mut Vec<ParseToken>) -> Result<(), &'static str>
{
    let token_list = out;
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
                token_list.push(ParseToken {token_type: LR_Type::TokenId as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: count as u32});
                i += count as usize - 1;
            }
            else if arg[i].is_ascii_digit()
            {
                let mut count = 1;
                while (i + count) != arg.len() && arg[i + count].is_ascii_digit()
                {
                    count += 1;
                }
                token_list.push(ParseToken {token_type: LR_Type::TokenNumber as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: count as u32});
                i += count as usize - 1;
            }
            else
            {
                match arg[i]
                {
                    b'+' => token_list.push(ParseToken {token_type: LR_Type::TokenPlus as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    b'-' => token_list.push(ParseToken {token_type: LR_Type::TokenMinus as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    b'*' => token_list.push(ParseToken {token_type: LR_Type::TokenTimes as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    b'/' => token_list.push(ParseToken {token_type: LR_Type::TokenDivide as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    b'=' => token_list.push(ParseToken {token_type: LR_Type::TokenEquals as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    b'(' => token_list.push(ParseToken {token_type: LR_Type::TokenOpen as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    b')' => token_list.push(ParseToken {token_type: LR_Type::TokenClose as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    b' ' => {}
                    b'\n' => {}
                    b'\r' => {}
                    b'\t' => {}
                    _ => {eprintln!("ERROR: Unknown char {}", arg[i] as char); return Err("Unknown value");}
                }
            }
            i += 1;
        }
        token_list.push(ParseToken {token_type: LR_Type::TokenEnd as i64, data: 0 as *const i8, length: 0});
    }
    return Ok(());
}



fn main()
{
    
    
    
    let mut bnf_src: Vec<u8>;
    {
        let file = File::open("./src/bnf.txt").unwrap();
        let mut buf_reader = BufReader::new(file);
        let mut bnf: String = String::new();
        buf_reader.read_to_string(&mut bnf).unwrap();
        bnf_src = bnf.into_bytes();
        bnf_src.push(0);
    }



    let mut table: [u8; 10000] = [0; 10000];
    let table_size: u32 = unsafe {write_parse_table_from_bnf(&mut table as *mut u8 as *mut c_void, 10000, bnf_src.as_ptr() as *const i8)};
    if table_size != 0
    {
        println!("failed to read table {}", table_size);
        return;
    }


    let mut expr: *mut Expr = 0 as *mut Expr;
    let mut msg: [u8; 1000] = [0; 1000];

    let mut map: HashMap<&[i8], *const Expr> = HashMap::new();
    loop 
    { 
        let mut read_str = String::new();
        io::stdin().read_line(&mut read_str).unwrap();
        read_str = read_str.trim_end().to_string();

        if read_str.as_str() == "q"
        {
            break;
        }
        let mut token_list: Vec<ParseToken> = Vec::new();
        let byte_vec = read_str.as_bytes().to_vec(); 
        {
            let err = string_to_tokens(&byte_vec, &mut token_list);
            if err.is_err()
            {
                continue;
            }
        }
        
        let token_count = token_list.len() as u32;
        let success: bool = unsafe {parse_bin(token_list.as_mut_ptr(), token_count, table.as_mut_ptr(), /*PRINT_EVERY_PARSE_STEP*/0, &mut expr, msg.as_mut_ptr() as *mut i8, 1000)};

        if success
        {
            unsafe {graphviz_from_syntax_tree(b"./input.dot\0".as_ptr() as *const i8, expr)};

            // TODO(Johan): add symbols to hash table before evaluating to handle variables and functions in a better way

            println!(" = {}", unsafe {eval_tree(expr, &mut map)});
        }
        else
        {
            println!("{}", std::str::from_utf8(&msg).unwrap());
        }       
    }
}