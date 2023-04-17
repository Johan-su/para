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


#[derive(Debug, PartialEq)]
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

unsafe fn get_expr_token_type(expr: *const Expr) -> LR_Type
{
    return i64_to_TokenType((*expr).token.token_type).unwrap();
}

unsafe fn slice_from_expr_token<'a>(expr: *const Expr) -> &'a[i8]
{
    return from_raw_parts((*expr).token.data, (*expr).token.length as usize);
}


unsafe fn eval_tree(expr: *const Expr, map: &mut HashMap<&[i8], Symbol>, in_func_call: Option<&(Func, f64)>) -> Result<f64, &'static str>
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
        LR_Type::TokenNumber => {return Ok(parse_number((*expr).token.data, (*expr).token.length))}
        LR_Type::TokenId => {todo!("not implemented")}
        LR_Type::TokenEnd => {panic!("unreachable")}
        LR_Type::ExpraltS => {
            if (*expr).expr_count != 1
            {
                panic!("unreachable");
            }
            return eval_tree(get_expr_in_array(expr, 0), map, in_func_call);
        }
        LR_Type::ExprFuncDecl => {panic!("unreachable")}
        LR_Type::ExprVarDecl => {panic!("unreachable")}
        LR_Type::ExprE => {
            match (*expr).expr_count
            {
                1 => {return eval_tree(get_expr_in_array(expr, 0), map, in_func_call)}
                2 => 
                {
                    let token = i64_to_TokenType((*get_expr_in_array(expr, 1)).token.token_type).unwrap(); 
                    match token 
                    {
                        LR_Type::TokenPlus => {return eval_tree(get_expr_in_array(expr, 0), map, in_func_call)}
                        LR_Type::TokenMinus => {return Ok(-(eval_tree(get_expr_in_array(expr, 0), map, in_func_call)?))}
                        _ => panic!("unexpected token")          
                    }
                }
                3 => 
                {
                    match i64_to_TokenType((*get_expr_in_array(expr, 1)).token.token_type).unwrap() 
                    {
                        LR_Type::TokenPlus =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map, in_func_call)?; 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map, in_func_call)?; 
                            return Ok(num1 + num2);
                        }
                        LR_Type::TokenMinus =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map, in_func_call)?; 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map, in_func_call)?; 
                            return Ok(num1 - num2);
                        }
                        LR_Type::TokenTimes =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map, in_func_call)?; 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map, in_func_call)?; 
                            return Ok(num1 * num2);
                        }
                        LR_Type::TokenDivide =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map, in_func_call)?; 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map, in_func_call)?; 
                            return Ok(num1 / num2);
                        }
                        LR_Type::ExprE =>
                        {
                            return eval_tree(get_expr_in_array(expr, 1), map, in_func_call);
                        }
                        _ => panic!("unreachable")
                    }
                }
                _ => panic!("unreachable")
            }
        }
        LR_Type::ExprFuncCall => 
        {
            assert_eq!((*expr).expr_count, 4);

            let func_name = get_expr_in_array(expr, 3);
            let open = get_expr_in_array(expr, 2);
            let call_expr = get_expr_in_array(expr, 1);
            let close = get_expr_in_array(expr, 0);

            assert_eq!(get_expr_token_type(func_name), LR_Type::TokenId);
            assert_eq!(get_expr_token_type(open), LR_Type::TokenOpen);
            assert_eq!(get_expr_token_type(call_expr), LR_Type::ExprE);
            assert_eq!(get_expr_token_type(close), LR_Type::TokenClose);

            let call_val: f64 = eval_tree(call_expr, map, in_func_call)?;
            
            let opt_func = map.get(slice_from_expr_token(func_name));
            if opt_func.is_some()
            {
                match opt_func.unwrap()
                {
                    Symbol::Func(x) => {return eval_tree(x.expr, map, Some(&(x.clone(), call_val)))}
                    _ => return Err("Symbol is not a function")
                }
            }
            else 
            {
                return Err("Undefined function");       
            }
        }
        LR_Type::ExprVar => 
        {
            assert_eq!((*expr).expr_count, 1);
            
            let slice: &[i8] = slice_from_expr_token(get_expr_in_array(expr, 0));            
            if in_func_call.is_some()
            {
                let tuple = in_func_call.unwrap();
                let func: &Func = &tuple.0;
                let val: f64 = tuple.1;
                

                //TODO(Johan): works with these prints, probably because of Undefined behavior from aliasing somewhere else.
                print_i8(func.var_name.as_ptr(), func.var_name.len() as u32);
                print_i8(slice.as_ptr(), slice.len() as u32);

                if func.var_name == slice
                {
                    return Ok(val);
                }
            }
            let val = map.get(slice);
            if val.is_none()
            {
                return Err("Undefined variable");
            }
            else
            {
                let var_expr: *const Expr = match val.unwrap() {Symbol::Var(x) => *x, _ => {return Err("Symbol is not a variable")}};
                return eval_tree(var_expr, map, in_func_call);    
            }
        }
    }
}



#[derive(Debug, Clone)]
struct Func<'a>
{
    var_name: &'a[i8],
    expr: *const Expr,
}

#[derive(Debug)]
enum Symbol<'a>
{
    Var(*const Expr),
    Func(Func<'a>),
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
                match arg[i] as char
                {
                    '+' => token_list.push(ParseToken {token_type: LR_Type::TokenPlus as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    '-' => token_list.push(ParseToken {token_type: LR_Type::TokenMinus as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    '*' => token_list.push(ParseToken {token_type: LR_Type::TokenTimes as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    '/' => token_list.push(ParseToken {token_type: LR_Type::TokenDivide as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    '=' => token_list.push(ParseToken {token_type: LR_Type::TokenEquals as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    '(' => token_list.push(ParseToken {token_type: LR_Type::TokenOpen as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    ')' => token_list.push(ParseToken {token_type: LR_Type::TokenClose as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1}),
                    ' ' => {}
                    '\n' => {}
                    '\r' => {}
                    '\t' => {}
                    _ => {eprintln!("ERROR: Unknown char {}", arg[i] as char); return Err("Unknown value");}
                }
            }
            i += 1;
        }
        token_list.push(ParseToken {token_type: LR_Type::TokenEnd as i64, data: 0 as *const i8, length: 0});
    }
    return Ok(());
}


unsafe fn add_declarations_if_needed_and_run(expr: *mut Expr, map: &mut HashMap<&[i8], Symbol>)
{
    if LR_Type::ExpraltS != get_expr_token_type(expr) || (*expr).expr_count == 0
    {
        return;
    }

    let decl_expr = get_expr_in_array(expr, 0);
    let token_type = get_expr_token_type(decl_expr);
    if LR_Type::ExprVarDecl == token_type
    {
        assert_eq!((*decl_expr).expr_count, 3);

        let var_name = get_expr_in_array(decl_expr, 2);
        let equals = get_expr_in_array(decl_expr, 1);
        let var_expr = get_expr_in_array(decl_expr, 0);


        assert_eq!(get_expr_token_type(var_name), LR_Type::TokenId);
        assert_eq!(get_expr_token_type(equals), LR_Type::TokenEquals);
        assert_eq!(get_expr_token_type(var_expr), LR_Type::ExprE);

        let slice = slice_from_expr_token(var_name);
        if map.insert(slice, Symbol::Var(var_expr)).is_some()
        {
            print!("redefined ");
            print_i8(slice.as_ptr(), slice.len() as u32);
        }
    }
    else if LR_Type::ExprFuncDecl == token_type
    {
        assert_eq!((*decl_expr).expr_count, 6);

        let func_name = get_expr_in_array(decl_expr, 5);
        let open_par = get_expr_in_array(decl_expr, 4);
        // expr with variable instead of just Var to make parsing work
        let var_expr = get_expr_in_array(decl_expr, 3);
        let close_par = get_expr_in_array(decl_expr, 2);
        let equals = get_expr_in_array(decl_expr, 1);
        let func_expr = get_expr_in_array(decl_expr, 0);


        assert_eq!(get_expr_token_type(func_name), LR_Type::TokenId);
        assert_eq!(get_expr_token_type(open_par), LR_Type::TokenOpen);
        assert_eq!(get_expr_token_type(var_expr), LR_Type::ExprE);
        assert_eq!(get_expr_token_type(close_par), LR_Type::TokenClose);
        assert_eq!(get_expr_token_type(equals), LR_Type::TokenEquals);
        assert_eq!(get_expr_token_type(func_expr), LR_Type::ExprE);

        let slice: &[i8] = slice_from_expr_token(func_name);
        let var_slice: &[i8] = slice_from_expr_token(get_expr_in_array(get_expr_in_array(var_expr, 0), 0));

        if map.insert(slice, Symbol::Func(Func {var_name: var_slice, expr: func_expr})).is_some()
        {
            print!("redefined ");
            print_i8(slice.as_ptr(), slice.len() as u32);
            println!();
        }
    }
    else if LR_Type::ExprE == token_type
    {
        let result = unsafe {eval_tree(expr, map, None)};
        match result
        {
            Ok(x) => println!(" = {}", x),    
            Err(x) => println!("{}", x),    
        }
    }
    else 
    {
        panic!("unreachable");    
    }     
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

    let mut map: HashMap<&[i8], Symbol> = HashMap::new();
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

            unsafe {add_declarations_if_needed_and_run(expr, &mut map)};


        }
        else
        {
            println!("{}", std::str::from_utf8(&msg).unwrap());
        }       
    }
}