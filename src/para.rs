#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(dead_code)]

mod ptg_header;
mod win_header;

use core::ffi::c_void;
use std::collections::HashMap;
use std::fs::File;
use std::io;
use std::io::BufReader;
use std::io::Read;
use std::mem::transmute;
use std::process::exit;
use std::io::Write;


use crate::ptg_header::*;
use crate::win_header::*;

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


unsafe fn print_expr_token(expr_token: *const Expr)
{
    print_i8((*expr_token).token.data, (*expr_token).token.length);
}



unsafe fn get_expr_in_array(expr: *const Expr, index: isize) -> *const Expr
{
    assert!((*expr).expr_count > index as u32);
    let exprs: *const *const Expr = transmute(expr.offset(1));
    return *exprs.offset(index);   
}

unsafe fn get_expr_token_type(expr: *const Expr) -> LR_Type
{
    return i64_to_TokenType((*expr).token.token_type).unwrap();
}

unsafe fn vec_from_expr_token(expr: *const Expr) -> Vec<char>
{
    let mut vec: Vec<char> = Vec::new();
    let data: *const i8 = (*expr).token.data;
    for i in 0..((*expr).token.length)
    {
        vec.push(*(data.offset(i as isize)) as u8 as char);
    }
    return vec;
}



unsafe fn eval_tree(expr: *const Expr, map: &mut HashMap<Vec<char>, Symbol>, in_func_call: Option<&(Func, f64)>, predefined_funcions: &HashMap<Vec<char>, fn(f64) -> f64>) -> Result<f64, &'static str>
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
            return eval_tree(get_expr_in_array(expr, 0), map, in_func_call, predefined_funcions);
        }
        LR_Type::ExprFuncDecl => {panic!("unreachable")}
        LR_Type::ExprVarDecl => {panic!("unreachable")}
        LR_Type::ExprE => {
            match (*expr).expr_count
            {
                1 => return eval_tree(get_expr_in_array(expr, 0), map, in_func_call, predefined_funcions),
                2 =>
                {
                    let token = i64_to_TokenType((*get_expr_in_array(expr, 1)).token.token_type).unwrap(); 
                    match token 
                    {
                        LR_Type::TokenPlus => {return eval_tree(get_expr_in_array(expr, 0), map, in_func_call, predefined_funcions)}
                        LR_Type::TokenMinus => {return Ok(-(eval_tree(get_expr_in_array(expr, 0), map, in_func_call, predefined_funcions)?))}
                        _ => panic!("unexpected token")          
                    }
                }
                3 => 
                {
                    match i64_to_TokenType((*get_expr_in_array(expr, 1)).token.token_type).unwrap() 
                    {
                        LR_Type::TokenPlus =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map, in_func_call, predefined_funcions)?; 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map, in_func_call, predefined_funcions)?; 
                            return Ok(num1 + num2);
                        }
                        LR_Type::TokenMinus =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map, in_func_call, predefined_funcions)?; 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map, in_func_call, predefined_funcions)?; 
                            return Ok(num1 - num2);
                        }
                        LR_Type::TokenTimes =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map, in_func_call, predefined_funcions)?; 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map, in_func_call, predefined_funcions)?; 
                            return Ok(num1 * num2);
                        }
                        LR_Type::TokenDivide =>
                        {
                            let num1 = eval_tree(get_expr_in_array(expr, 2), map, in_func_call, predefined_funcions)?; 
                            let num2 = eval_tree(get_expr_in_array(expr, 0), map, in_func_call, predefined_funcions)?; 
                            return Ok(num1 / num2);
                        }
                        LR_Type::ExprE =>
                        {
                            return eval_tree(get_expr_in_array(expr, 1), map, in_func_call, predefined_funcions);
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

            let call_val: f64 = eval_tree(call_expr, map, in_func_call, predefined_funcions)?;

            let pre_func = predefined_funcions.get(&vec_from_expr_token(func_name));
            if pre_func.is_some()
            {
                return Ok(pre_func.unwrap()(call_val));
            }

            let opt_func = map.get(&vec_from_expr_token(func_name));
            if opt_func.is_some()
            {
                match opt_func.unwrap()
                {
                    Symbol::Func(x) => {return eval_tree(x.expr, map, Some(&(x.clone(), call_val)), predefined_funcions)}
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
            let vec: Vec<char> = vec_from_expr_token(get_expr_in_array(expr, 0));
            if in_func_call.is_some()
            {
                let tuple = in_func_call.unwrap();
                let func: &Func = &tuple.0;
                let val: f64 = tuple.1;
                

                if func.var_name == vec
                {
                    return Ok(val);
                }
            }
            let val = map.get(&vec);
            if val.is_none()
            {
                return Err("Undefined variable");
            }
            else
            {
                let var_expr: *const Expr = match val.unwrap() {Symbol::Var(x) => *x, _ => {return Err("Symbol is not a variable")}};
                return eval_tree(var_expr, map, in_func_call, predefined_funcions);    
            }
        }
    }
}



#[derive(Debug, Clone)]
struct Func
{
    var_name: Vec<char>,
    expr: *const Expr,
}

#[derive(Debug)]
enum Symbol
{
    Var(*const Expr),
    Func(Func),
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


unsafe fn add_declarations_if_needed_and_run(expr: *mut Expr, map: &mut HashMap<Vec<char>, Symbol>, predefined_funcions: &HashMap<Vec<char>, fn(f64) -> f64>)
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

        let name_vec = vec_from_expr_token(var_name);
        if map.insert(name_vec, Symbol::Var(var_expr)).is_some()
        {
            print!("redefined ");
            print_expr_token(var_name);
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


        let func_vec: Vec<char> = vec_from_expr_token(func_name);

        let var: *const Expr = get_expr_in_array(var_expr, 0);
        if get_expr_token_type(var) != LR_Type::ExprVar
        {
            println!("Function argument has to be a single variable");
            return;
        }
        let var_value: *const Expr = get_expr_in_array(var, 0);
        assert_eq!(get_expr_token_type(var_value), LR_Type::TokenId);

        let var_vec: Vec<char> = vec_from_expr_token(var_value);


        if predefined_funcions.get(&func_vec).is_some()
        {
            println!("Cannot redefine predefined functions");
            return;
        }

        if map.insert(func_vec, Symbol::Func(Func {var_name: var_vec, expr: func_expr})).is_some()
        {
            print!("redefined ");
            print_expr_token(func_name);
        }
        else 
        {
            print!("defined ");
            print_expr_token(func_name);
        }
    }
    else if LR_Type::ExprE == token_type
    {
        let result = eval_tree(expr, map, None, predefined_funcions);
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



unsafe fn print_tree(expr: *const Expr)
{
    let token_type: i64 = (*expr).token.token_type;
    let token_length = (*expr).token.length;
    print!("token_type = {}, length = {}, ptr = {:#x} val = ", token_type, token_length, (*expr).token.data as usize);
    print_expr_token(expr);
    for i in 0..((*expr).expr_count)
    {
        let arr_expr: *const Expr = get_expr_in_array(expr, i as isize);
        print_tree(arr_expr);
    }
}


























enum Mode
{
    Normal,
    Insert,
}





fn begin_esc() -> bool
{
    let console: HANDLE = unsafe {GetStdHandle(STD_OUTPUT_HANDLE)};

    let mut mode: DWORD = 0;
    if unsafe {GetConsoleMode(console, &mut mode)} == FALSE
    {
        return false;
    }
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT; 
    if unsafe {SetConsoleMode(console, mode)} == FALSE
    {
        return false;
    }
    return true;
}

fn end_esc() -> bool
{
    io::stdout().flush().unwrap();
    let console: HANDLE = unsafe {GetStdHandle(STD_OUTPUT_HANDLE)};
    let mut mode: DWORD = 0;
    if unsafe {GetConsoleMode(console, &mut mode)} == FALSE { return false }

    mode &= !ENABLE_VIRTUAL_TERMINAL_PROCESSING & !ENABLE_PROCESSED_OUTPUT; 

    if unsafe {SetConsoleMode(console, mode)} == FALSE { return false }
    return true
}

unsafe fn revert_console(out_console: HANDLE, in_console: HANDLE, old_out_mode: DWORD, old_in_mode: DWORD)
{
    begin_esc();
    // change back to main buffer
    print!("\x1b[?1049l");
    end_esc();
    SetConsoleMode(out_console, old_out_mode);
    SetConsoleMode(in_console, old_in_mode);
}


static mut y_shift: i16 = 0;

fn begin_ui()
{
    begin_esc();
    // clear_screen_esc();
    end_esc();
    unsafe {y_shift = 0}

}

fn end_ui()
{
    unsafe {y_shift = 0}
}


fn input_string_box(name: &'static str, str_buffer: &mut [u8], max_len: usize) -> bool
{
    let x = 1;
    let y = 1;
    let height = 3;
    let width = max_len;
    let mut str: String = String::new();
    for _ in 0..width
    {
        str += "-"; 
    }
    begin_esc();
    let x_bound: bool; 
    let y_bound: bool; 
    unsafe
    {
        assert!(max_len <= i16::MAX as usize);
        x_bound = cursor.x >= x && cursor.x <= x + (width - 1) as i16;
        y_bound = cursor.y >= y && cursor.y <= y + (height - 1);
        write_at_pos_esc(&str, x, y + y_shift);
        write_at_pos_esc(&str, x, y + (height - 1) + y_shift);
        y_shift += height;
    }
    end_esc();
    if x_bound && y_bound
    {
        return true;
    }
    return false;
}

fn get_cursor_position_esc() -> (i16, i16)
{
    return unsafe {(cursor.x, cursor.y)};
}

struct Cursor
{
    x: i16,
    y: i16,


    stack_count: usize, 
    save_stack: [(i16, i16); 8],

}
static mut cursor: Cursor = Cursor {x: 1, y: 1, stack_count: 0, save_stack: [(1, 1); 8]};

fn save_cursor_position()
{
    unsafe
    {
        assert!(cursor.stack_count < 8);
        cursor.save_stack[cursor.stack_count] = (cursor.x, cursor.y);
        cursor.stack_count += 1;
    }
}

fn move_cursor_esc(x: i16, y: i16)
{
    unsafe {set_cursor_to_esc(cursor.x + x, cursor.y + y)};
}


fn restore_cursor_position_esc()
{
    unsafe
    {
        assert!(cursor.stack_count > 0);
        cursor.stack_count -= 1;
        let saved_cursor = cursor.save_stack[cursor.stack_count];
        set_cursor_to_esc(saved_cursor.0, saved_cursor.1);
    }
}

fn clear_screen_esc()
{
    print!("\x1b[2J");
}

fn set_cursor_to_esc(x: i16, y: i16)
{
    let mut x = x;
    let mut y = y;
    if x < 1 { x = 1; } ;
    if y < 1 { y = 1; } ; 

    unsafe
    {
        cursor.x = x;
        cursor.y = y;
    }

    print!("\x1b[{};{}H", y, x);
}

fn write_at_pos_esc(str: &String, x: i16, y: i16)
{
    save_cursor_position();
    set_cursor_to_esc(x, y);

    end_esc(); // ignore terminal sequences from user input text
    print!("{}", str);
    begin_esc();

    restore_cursor_position_esc();
}

fn main()
{

    let mut old_stdin_mode: DWORD = 0;
    let mut old_stdout_mode: DWORD = 0;

    unsafe
    {
        let stdin: HANDLE = GetStdHandle(STD_INPUT_HANDLE);
        if stdin == INVALID_HANDLE_VALUE
        {
            exit(420);
        }

        let stdout: HANDLE = GetStdHandle(STD_OUTPUT_HANDLE);
        if stdout == INVALID_HANDLE_VALUE
        {
            exit(420);
        }
        GetConsoleMode(stdin, &mut old_stdin_mode);
        GetConsoleMode(stdout, &mut old_stdout_mode);

        
        if SetConsoleMode(stdin, 0) == FALSE
        {
            exit(421);
        }
        
        
        begin_esc();
        // change to alternate buffer
        print!("\x1b[?1049h");
        set_cursor_to_esc(1, 1);
        end_esc();

        
        let mut x: SHORT = 0;
        let mut y: SHORT = 0;
        let mut w: SHORT = 0;
        let mut h: SHORT = 0;

        // let mut input_box_count: usize = 1; 
        loop
        {
            let mut screen_buf_info: CONSOLE_SCREEN_BUFFER_INFO = std::mem::MaybeUninit::zeroed().assume_init();
            if GetConsoleScreenBufferInfo(stdout, &mut screen_buf_info as PCONSOLE_SCREEN_BUFFER_INFO) == FALSE
            {
                eprintln!("ERROR: GetConsoleScreenBufferInfo {}", GetLastError());
                exit(-1);
            }
            

            let new_x = screen_buf_info.srWindow.Left;
            let new_y = screen_buf_info.srWindow.Top;
            let new_w = screen_buf_info.srWindow.Right - screen_buf_info.srWindow.Left;
            let new_h = screen_buf_info.srWindow.Bottom - screen_buf_info.srWindow.Top;

            if x != new_x || y != new_y || w != new_w || h != new_h
            {
                x = new_x;
                y = new_y;
                w = new_w;
                h = new_h;
                // println!("terminal x = {}, y = {}, w = {}, h = {}", x, y, w, h);
            }

            let mut buf: Vec<u8> = Vec::new();

            begin_ui();
            let inside: bool = input_string_box("1", buf.as_mut_slice(), 16);
            end_ui();

            begin_esc();
            write_at_pos_esc(&format!("pos = [ {} {} ] {}", cursor.x, cursor.y, inside), 0, h);
            end_esc();

            // get_cursor_position_esc();

            let mut buf: INPUT_RECORD = std::mem::MaybeUninit::zeroed().assume_init();
            let num_char_to_read: DWORD = 1;
            let mut num_chars_read: DWORD = 0;
            if ReadConsoleInputW(stdin, &mut buf as PINPUT_RECORD, num_char_to_read, &mut num_chars_read) == FALSE
            {
                exit(-1)
            }

            match buf.EventType 
            {
                FOCUS_EVENT => {},
                KEY_EVENT => 
                {
                    let key_event = buf.event.KeyEvent; 
                    let key_code = key_event.wVirtualKeyCode;
                    if key_event.bKeyDown == TRUE
                    {
                        if key_code == VK_UP
                        {
                            begin_esc();
                            move_cursor_esc(0, -1);
                            end_esc();
                        }
                        if key_code == VK_DOWN
                        {
                            begin_esc();
                            move_cursor_esc(0, 1);
                            end_esc();
                        }
                        if key_code == VK_RIGHT
                        {
                            begin_esc();
                            move_cursor_esc(1, 0);
                            end_esc();
                        }
                        if key_code == VK_LEFT
                        {
                            begin_esc();
                            move_cursor_esc(-1, 0);
                            end_esc();
                        }
                        if key_code == 'S' as u16
                        {
                            begin_esc();
                            write_at_pos_esc(&"save".to_string(), 1, h);
                            end_esc();
                        }
                        if key_code == 'R' as u16
                        {
                            begin_esc();
                            write_at_pos_esc(&"restore".to_string(), 1, h);
                            end_esc();
                        }
                        if key_code == 'Q' as u16
                        {
                            if key_event.dwControlKeyState == LEFT_ALT_PRESSED
                            {
                                break;
                            }
                        }
                        else if key_code == 'C' as u16
                        {
                            if key_event.dwControlKeyState == LEFT_CTRL_PRESSED
                            {
                                break;
                            }
                        }
                        else
                        {
                            // println!("code = {:#x}", key_code);
                        }
                    }
                },
                MENU_EVENT => {},
                MOUSE_EVENT => {},
                WINDOW_BUFFER_SIZE_EVENT => {},
                _ => {panic!("unreachable")}
            }
            io::stdout().flush().unwrap();

    
        }
        revert_console(stdout, stdin, old_stdout_mode, old_stdin_mode);
        exit(0);
    }
}























fn main2()
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



    let mut predefined_functions: HashMap<Vec<char>, fn(f64) -> f64> = HashMap::new();

    predefined_functions.insert("sin".chars().collect(), f64::sin);
    predefined_functions.insert("cos".chars().collect(), f64::cos);
    predefined_functions.insert("tan".chars().collect(), f64::tan);

    predefined_functions.insert("asin".chars().collect(), f64::asin);
    predefined_functions.insert("acos".chars().collect(), f64::acos);
    predefined_functions.insert("atan".chars().collect(), f64::atan);

    predefined_functions.insert("sqrt".chars().collect(), f64::sqrt);
    predefined_functions.insert("cbrt".chars().collect(), f64::cbrt);

    predefined_functions.insert("exp".chars().collect(), f64::exp);

    predefined_functions.insert("ln".chars().collect(), f64::ln);
    predefined_functions.insert("log2".chars().collect(), f64::log2);
    predefined_functions.insert("log10".chars().collect(), f64::log10);

    predefined_functions.insert("floor".chars().collect(), f64::floor);
    predefined_functions.insert("ceil".chars().collect(), f64::ceil);
    
    predefined_functions.insert("abs".chars().collect(), f64::abs);


    let mut map: HashMap<Vec<char>, Symbol> = HashMap::new();

    let mut byte_vectors: Vec<Vec<u8>> = Vec::new();
    const size: usize = 4096;
    byte_vectors.reserve_exact(size);

    let mut i: usize = 0;

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
        assert_ne!(i, size);
        byte_vectors.insert(i, read_str.as_bytes().to_vec()); 
        {
            let err = string_to_tokens(&byte_vectors.get(i).unwrap(), &mut token_list);
            if err.is_err()
            {
                continue;
            }
        }


        let token_count = token_list.len() as u32;
        let success: bool = unsafe {parse_bin(token_list.as_mut_ptr(), token_count, table.as_mut_ptr(), /*PRINT_EVERY_PARSE_STEP*/0, &mut expr, msg.as_mut_ptr() as *mut i8, 1000)};

        if success
        {
            i += 1;
            unsafe {graphviz_from_syntax_tree(b"./input.dot\0".as_ptr() as *const i8, expr)};
            
            // unsafe {print_tree(expr)};
            unsafe {add_declarations_if_needed_and_run(expr, &mut map, &predefined_functions)};


        }
        else
        {
            println!("{}", std::str::from_utf8(&msg).unwrap());
        }       
    }
}