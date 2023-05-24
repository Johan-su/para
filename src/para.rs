#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(dead_code)]

mod ptg_header;
mod win_header;

use core::ffi::c_void;
use core::hash::Hash;
use std::collections::HashMap;
use std::io;
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

unsafe fn parse_number(data: *const i8, data_length: u32, stride: u8) -> f64
{
    let mut result: f64 = 0.0;
    let mut num_pos: f64 = 1.0;
    for i in (0..data_length).rev()
    {
        let val = *(data.offset(i as isize * stride as isize)) as u8 as char;
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

unsafe fn expr_token_to_string(expr: *const Expr) -> String
{
    let mut vec: Vec<char> = Vec::new();

    let data: *const i8 = (*expr).token.data;
    let data_length: u32 = (*expr).token.length;
    let stride: u8 = (*expr).token.stride;
    {
        for i in 0..data_length
        {
            vec.push(*(data.offset(i as isize * stride as isize)) as u8 as char);
        }
    }
    return vec.into_iter().collect();
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

unsafe fn str_view_from_expr_token(expr: *const Expr) -> String_View
{
    return String_View { data: (*expr).token.data, stride: (*expr).token.stride as usize, length: (*expr).token.length as usize }
}
fn str_view_from_str(str: &'static str) -> String_View
{
    let cstr: String_View = String_View { data: str as *const str as *const i8, stride: 1, length: str.len()};
    return cstr;
}


unsafe fn eval_tree(expr: *const Expr, map: &mut HashMap<String_View, Symbol>, in_func_call: Option<&(Func, f64)>, predefined_funcions: &HashMap<String_View, fn(f64) -> f64>) -> Result<f64, &'static str>
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
        LR_Type::TokenNumber => {return Ok(parse_number((*expr).token.data, (*expr).token.length, (*expr).token.stride))}
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

            let func_name_str: String_View = str_view_from_expr_token(func_name);

            let pre_func = predefined_funcions.get(&func_name_str);
            if pre_func.is_some()
            {
                return Ok(pre_func.unwrap()(call_val));
            }

            let opt_func = map.get(&func_name_str);
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
            let var_name_str: String_View = str_view_from_expr_token(get_expr_in_array(expr, 0));
            if in_func_call.is_some()
            {
                let tuple = in_func_call.unwrap();
                let func: &Func = &tuple.0;
                let val: f64 = tuple.1;


                if func.var_name_str == var_name_str
                {
                    return Ok(val);
                }
            }
            let val = map.get(&var_name_str);
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
    var_name_str: String_View,
    expr: *const Expr,
}

#[derive(Debug)]
enum Symbol
{
    Var(*const Expr),
    Func(Func),
}

#[derive(Debug, Eq, Clone, Copy)]
struct String_View
{
    data: *const i8,
    stride: usize,
    length: usize,
}

impl Hash for String_View
{
    fn hash<H: std::hash::Hasher>(&self, state: &mut H)
    {
        self.length.hash(state);
        for i in 0..self.length
        {
            unsafe {*self.data.offset((i * self.stride) as isize)}.hash(state);
        }
    }
}
impl PartialEq for String_View
{
    fn eq(&self, other: &Self) -> bool {
        if self.length != other.length
        {
            return false;
        }

        for i in 0..self.length as isize
        {
            if unsafe {*self.data.offset(i * self.stride as isize) != *other.data.offset(i * other.stride as isize)}
            {
                return false
            }
        }
        return true;
    }
}


fn string_to_tokens<'a>(arg: &'a [char], str_len: usize, out: &'a mut Vec<ParseToken>) -> Result<(), String>
{
    let token_list = out;
    {
        let mut i: usize = 0;
        while i < str_len
        {
            if arg[i].is_ascii_alphabetic()
            {
                let mut count = 1;
                while (i + count) != str_len && arg[i + count].is_ascii_alphanumeric()
                {
                    count += 1;
                }
                token_list.push(ParseToken {token_type: LR_Type::TokenId as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: count as u32, stride: 4});
                i += count as usize - 1;
            }
            else if arg[i].is_ascii_digit()
            {
                let mut count = 1;
                while (i + count) != str_len && arg[i + count].is_ascii_digit()
                {
                    count += 1;
                }
                token_list.push(ParseToken {token_type: LR_Type::TokenNumber as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: count as u32, stride: 4});
                i += count as usize - 1;
            }
            else
            {
                match arg[i] as char
                {
                    '+' => token_list.push(ParseToken {token_type: LR_Type::TokenPlus as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1, stride: 4}),
                    '-' => token_list.push(ParseToken {token_type: LR_Type::TokenMinus as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1, stride: 4}),
                    '*' => token_list.push(ParseToken {token_type: LR_Type::TokenTimes as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1, stride: 4}),
                    '/' => token_list.push(ParseToken {token_type: LR_Type::TokenDivide as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1, stride: 4}),
                    '=' => token_list.push(ParseToken {token_type: LR_Type::TokenEquals as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1, stride: 4}),
                    '(' => token_list.push(ParseToken {token_type: LR_Type::TokenOpen as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1, stride: 4}),
                    ')' => token_list.push(ParseToken {token_type: LR_Type::TokenClose as i64, data: unsafe {arg.as_ptr().offset(i as isize) as *const i8}, length: 1, stride: 4}),
                    ' ' => {}
                    '\n' => {}
                    '\r' => {}
                    '\t' => {}
                    _ => return Err(format!("ERROR: Unknown char {}", arg[i]))
                }
            }
            i += 1;
        }
        token_list.push(ParseToken {token_type: LR_Type::TokenEnd as i64, data: 0 as *const i8, length: 0, stride: 4});
    }
    return Ok(());
}


unsafe fn add_var_decl(decl_expr: *const Expr, map: &mut HashMap<String_View, Symbol>) -> String 
{
    assert_eq!((*decl_expr).expr_count, 3);

    let var_name = get_expr_in_array(decl_expr, 2);
    let equals = get_expr_in_array(decl_expr, 1);
    let var_expr = get_expr_in_array(decl_expr, 0);


    assert_eq!(get_expr_token_type(var_name), LR_Type::TokenId);
    assert_eq!(get_expr_token_type(equals), LR_Type::TokenEquals);
    assert_eq!(get_expr_token_type(var_expr), LR_Type::ExprE);

    let name_str = str_view_from_expr_token(var_name);
    if map.insert(name_str, Symbol::Var(var_expr)).is_some()
    {
        return format!("Redefined {}", expr_token_to_string(var_name));
    }
    return format!("Defined {}", expr_token_to_string(var_name));
}

unsafe fn add_func_decl(decl_expr: *const Expr, map: &mut HashMap<String_View, Symbol>, predefined_functions: &HashMap<String_View, fn(f64) -> f64>) -> Result<String, String>
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



    let func: String_View = str_view_from_expr_token(func_name);

    let var: *const Expr = get_expr_in_array(var_expr, 0);
    if get_expr_token_type(var) != LR_Type::ExprVar
    {
        return Err("Function argument has to be a single variable".to_string());
    }
    let var_value: *const Expr = get_expr_in_array(var, 0);
    assert_eq!(get_expr_token_type(var_value), LR_Type::TokenId);

    let var_str: String_View = str_view_from_expr_token(var_value);


    if predefined_functions.get(&func).is_some()
    {
        return Err("Cannot redefine predefined functions".to_string());
    }

    if map.insert(func, Symbol::Func(Func {var_name_str: var_str, expr: func_expr})).is_some()
    {
        return Ok(format!("Redefined {}", expr_token_to_string(func_name)));
    }
    else
    {
        return Ok(format!("Defined {}", expr_token_to_string(func_name)));
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
    assert!(unsafe {!debug_escape});
    unsafe {debug_escape = true};
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
    assert!(unsafe {debug_escape});
    unsafe {debug_escape = false};
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

fn str_as_usize(str: &'static str) -> usize
{
    return str as *const str as *const () as usize;
}

fn is_active(unique_id: usize) -> bool
{
    unsafe
    {
        if active.stack_count == 0
        {
            return false;
        }
        return active.stack[active.stack_count - 1].index == unique_id;
    }
}

fn is_hot(unique_id: usize) -> bool
{
    return unsafe {hot.index} == unique_id;
}

// https://www.youtube.com/watch?v=Z1qyvQsjK5Y

fn inside(x: usize, y: usize, width: usize, height: usize) -> bool
{
    let cx = unsafe {cursor.x} as usize;
    let cy = unsafe {cursor.y} as usize;

    let bx: bool = cx >= x && cx < x + width;
    let by: bool = cy >= y && cy < y + height;

    return bx && by;
}


fn output_string_box(screen: &mut Terminal_Screen, unique_id: usize, str_buffer: &[char], max_len: usize)
{
    assert!(max_len <= i16::MAX as usize);

    let height = OUTPUT_STR_HEIGHT;
    let width = max_len;


    let xy = get_xy_with_current_layout(width, height);
    let x: usize = xy.0;
    let y: usize = xy.1;

    if is_active(unique_id)
    {
    }
    else if is_hot(unique_id)
    {
    }
    if inside(x, y, width, height)
    {
        unsafe
        {
            hot.owner = hot.index;
            hot.item = Ui_kind::OUTPUT_TEXT;
            hot.index = unique_id;
        }
    }

    write_string_at_pos(screen, str_buffer, 255, 255, 255, x as i16, y as i16);
}

#[derive(Clone, Copy)]
enum Flow
{
    INVALID,

    UPPER_LEFT_FLOW_RIGHT,
    UPPER_LEFT_FLOW_DOWN,

    UPPER_RIGHT_FLOW_LEFT,
    UPPER_RIGHT_FLOW_DOWN,

    BOTTOM_LEFT_FLOW_RIGHT,
    BOTTOM_LEFT_FLOW_UP,
    
    BOTTOM_RIGHT_FLOW_LEFT,
    BOTTOM_RIGHT_FLOW_UP,
}



#[derive(Clone, Copy)]
struct Ui_Layout
{
    x: usize,
    y: usize,
    w: usize,
    h: usize,
    flow: Flow,
}


fn ui_box(screen: &mut Terminal_Screen, unique_id: usize, layout: &mut Ui_Layout, inputs: &Input)
{
    let _ = screen; // pass screen incase we want to render different when in hot/active vs not active
    let _ = inputs;
    if is_active(unique_id)
    {

    }
    else if is_hot(unique_id)
    {

    }
    if inside(layout.x, layout.y, layout.w, layout.h)
    {
        unsafe
        {
            hot.owner = hot.index;
            hot.item = Ui_kind::BOX;
            hot.index = unique_id;
        }
    }
    layout.x += 1;
    layout.y += 1;
    layout.w -= 1;
    layout.h -= 1;
}


fn str_buffer_len(str_buffer: &[char], max_len: usize) -> usize
{
    assert!(str_buffer.len() >= max_len);
    for i in 0..max_len
    {
        if str_buffer[i] == '\0'
        {
            return i;
        }
    }
    return max_len;
}


enum Input_Box_Actions
{
    None,
    LEAVE_WITH_ENTER,
    LEAVE_WITH_ESCAPE,
}


fn input_string_box(screen: &mut Terminal_Screen, unique_id: usize, str_buffer: &mut [char], max_len: usize, inputs: &Input) -> Input_Box_Actions
{
    fn shift_string_right_from_index(buffer: &mut [char], max_len: usize, index: usize)
    {
        assert!(index < max_len);

        let str_len = str_buffer_len(buffer, max_len); 
        if str_len < max_len
        {
            for i in (index..str_len + 1).rev()
            {
                buffer[i] = buffer[i - 1];
            }
        }
    }

    fn shift_string_left_from_index(buffer: &mut [char], max_len: usize, index: usize)
    {
        assert!(index < max_len);


        let str_len = str_buffer_len(buffer, max_len);
        assert!(str_len > 0);

        {

            for i in index..str_len - 1
            {
                buffer[i] = buffer[i + 1];
            }
        }
        buffer[str_len - 1] = '\0';
    }


    let mut result = Input_Box_Actions::None;

    let height = INPUT_STR_HEIGHT;
    let width = max_len;

    let tup = get_xy_with_current_layout(width, height);
    let x: usize = tup.0;
    let y: usize = tup.1;






    let mut r: u8 = 255;
    let mut g: u8 = 255;
    let mut b: u8 = 255;


    if is_active(unique_id)
    {
        r = 154;
        g = 205;
        b = 50;
        begin_esc();
        if inputs.key_val == VK_LEFT
        {
            if unsafe {cursor.x} > x as i16
            {
                move_cursor_esc(-1, 0);
            }
        }
        else if inputs.key_val == VK_RIGHT
        {
            if unsafe {cursor.x as usize} < str_buffer_len(str_buffer, max_len) + 1
            {
                move_cursor_esc(1, 0);
            }
        }
        else if inputs.key_val == VK_ESCAPE
        {
            let _id = pop_active();
            result = Input_Box_Actions::LEAVE_WITH_ESCAPE;
        }
        else if inputs.key_val == VK_RETURN
        {
            let _id = pop_active();
            result =  Input_Box_Actions::LEAVE_WITH_ENTER;
        }
        else if inputs.key_val == VK_BACK
        {
            let x_in_str = unsafe {cursor.x} as usize - x;

            if x_in_str <= 0
            {
                // do nothing
            }
            else if x_in_str == max_len
            {
                str_buffer[max_len - 1] = '\0';
                move_cursor_esc(-1, 0);
            }
            else
            {
                shift_string_left_from_index(str_buffer, max_len, x_in_str - 1);
                move_cursor_esc(-1, 0);
            }
        }
        else if inputs.unicode_char != 0
        {
            let x_in_str = unsafe {cursor.x} as usize - x;

            if str_buffer_len(str_buffer, max_len) == max_len
            {
                // do nothing
            }
            else if x_in_str == 0 
            {
                shift_string_right_from_index(str_buffer, max_len, 1);
                str_buffer[x_in_str] = char::from_u32(inputs.unicode_char as u32).unwrap();
                move_cursor_esc(1, 0);
            }
            else if x_in_str < max_len
            {
                shift_string_right_from_index(str_buffer, max_len, x_in_str);
                str_buffer[x_in_str] = char::from_u32(inputs.unicode_char as u32).unwrap();
                move_cursor_esc(1, 0);
            }
        }
        end_esc();
    }
    else if is_hot(unique_id)
    {
        if inputs.key_val == 'A' as u16
        {
            push_active(unsafe {hot});
            begin_esc();
            set_cursor_to_esc((x + str_buffer_len(str_buffer, max_len)) as i16, y as i16 + 1);
            end_esc();
        }

        r = 255;
        g = 255;
        b = 0;
    }

    if inside(x, y, width, height)
    {
        unsafe
        {
            hot.owner = hot.index;
            hot.item = Ui_kind::INPUT_TEXT;
            hot.index = unique_id;
        }
    }


    assert!(max_len <= i16::MAX as usize);
    for i in 0..width
    {
        write_char_at_pos(screen, '-', r, g, b, (x + i) as i16, y as i16);
        write_char_at_pos(screen, '-', r, g, b, (x + i) as i16, (y + 2) as i16);
    }
    write_string_at_pos(screen, str_buffer, 255, 255, 255, x as i16, (y + 1) as i16);

    return result;
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

#[derive(Debug, Clone, Copy)]
enum Ui_kind
{
    None,
    INPUT_TEXT,
    OUTPUT_TEXT,
    BOX,
    SCROLL_LIST,
}

#[derive(Debug, Clone, Copy)]
struct Ui_id
{
    owner: usize,
    item: Ui_kind,
    index: usize,
}


const INPUT_STR_HEIGHT: usize = 3;
const OUTPUT_STR_HEIGHT: usize = 1;


#[derive(Debug)]
struct Active_Stack
{
    stack_count: usize,
    stack: [Ui_id; 8],
}

struct Layout_Stack
{
    stack_count: usize,
    stack: [Ui_Layout; 16],
}

static mut cursor: Cursor = Cursor {x: 0, y: 0, stack_count: 0, save_stack: [(0, 0); 8]};
// static mut last_input: u16 = 0;

static mut hot: Ui_id = Ui_id {owner: 0, item: Ui_kind::None, index: 0};
static mut active: Active_Stack = Active_Stack {stack_count: 0, stack: [Ui_id {owner: 0, item: Ui_kind::None, index: 0}; 8]};
static mut layout_stack: Layout_Stack = Layout_Stack {
    stack_count: 0, 
    stack: [Ui_Layout {x: 0, y: 0, w: 0, h: 0, flow: Flow::INVALID}; 16]
};
static mut debug_escape: bool = false;

fn push_active(id: Ui_id)
{
    unsafe
    {
        assert!(active.stack_count < active.stack.len());
        active.stack[active.stack_count] = id;
        active.stack_count += 1;
    }
}

fn pop_active() -> Ui_id
{
    unsafe
    {
        assert!(active.stack_count > 0);
        active.stack_count -= 1;
        return active.stack[active.stack_count];
    }
}


fn push_layout(layout: Ui_Layout)
{
    unsafe
    {
        // ignore if no parent
        if layout_stack.stack_count != 0
        {
            let parent = layout_stack.stack[layout_stack.stack_count - 1];
            let bx: bool = layout.x + layout.w >= parent.x && layout.x <= parent.x + parent.w;
            let by: bool = layout.y + layout.h >= parent.y && layout.y <= parent.y + parent.h;

            if !(bx && by)
            {
                assert!(false, "child layout is out of parent layout's bounds") //TODO(Johan): change to real error
            }
        }


        assert!(layout_stack.stack_count < layout_stack.stack.len());
        layout_stack.stack[layout_stack.stack_count] = layout;
        layout_stack.stack_count += 1;
    } 
}

fn pop_layout() -> Ui_Layout
{
    unsafe
    {
        assert!(layout_stack.stack_count > 0);
        layout_stack.stack_count -= 1;
        return layout_stack.stack[layout_stack.stack_count];
    }
}

fn get_current_layout() -> &'static mut Ui_Layout
{
    unsafe 
    {
        assert!(layout_stack.stack_count > 0);
        return &mut layout_stack.stack[layout_stack.stack_count - 1];
    }
}

fn get_xy_with_current_layout(width: usize, height: usize) -> (usize, usize)
{
    let layout: &mut Ui_Layout = get_current_layout();
    
    assert!(layout.w >= width);
    assert!(layout.h >= height);
    
    let x: usize;
    let y: usize;
    match layout.flow
    {
        Flow::INVALID => {panic!("unreachable")},
        Flow::UPPER_LEFT_FLOW_RIGHT => 
        {

            x = layout.x;
            y = layout.y;
            layout.x += width;

        },
        Flow::UPPER_LEFT_FLOW_DOWN => 
        {
            x = layout.x;
            y = layout.y;
            layout.y += height;
        },
        Flow::UPPER_RIGHT_FLOW_LEFT => 
        {
            x = layout.x + layout.w - width;
            y = layout.y;
            layout.w -= width;
        },
        Flow::UPPER_RIGHT_FLOW_DOWN => 
        {
            x = layout.x + layout.w - width;
            y = layout.y;
            layout.y += height;
        },
        Flow::BOTTOM_LEFT_FLOW_RIGHT => 
        {
            x = layout.x;
            y = layout.y + layout.h - height;
            layout.x += width;
        },
        Flow::BOTTOM_LEFT_FLOW_UP => 
        {
            x = layout.x;
            y = layout.y + layout.h - height;
            layout.h -= height;
        },
        Flow::BOTTOM_RIGHT_FLOW_LEFT => 
        {
            x = layout.x + layout.w - width;
            y = layout.y + layout.h - height;
            layout.w -= width;
        },
        Flow::BOTTOM_RIGHT_FLOW_UP => 
        {
            x = layout.x + layout.w - width;
            y = layout.y + layout.h - height;
            layout.h -= height;
        },
    }
    return (x, y);
}




#[derive(Debug)]
struct Screen_Element
{
    character: char,
    r: u8,
    g: u8,
    b: u8,
}


#[derive(Debug)]
struct Terminal_Screen
{
    buffer: Vec<Screen_Element>,
    width: usize,
    height: usize,
}


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
    assert!(unsafe {debug_escape});
    unsafe {set_cursor_to_esc(cursor.x + x, cursor.y + y)};
}


fn restore_cursor_position_esc()
{
    assert!(unsafe {debug_escape});
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
    assert!(unsafe {debug_escape});
    print!("\x1b[2J");
}

fn set_cursor_to_esc(x: i16, y: i16)
{
    assert!(unsafe {debug_escape});
    let mut x = x;
    let mut y = y;
    if x < 0 { x = 0; };
    if y < 0 { y = 0; };

    unsafe
    {
        cursor.x = x;
        cursor.y = y;
    }

    print!("\x1b[{};{}H", y + 1, x + 1);
}



fn present_back_terminal_buffer()
{
    let screen: &mut Terminal_Screen = unsafe {&mut front_buffer};
    let back_screen: &mut Terminal_Screen = unsafe {&mut back_buffer};
    save_cursor_position();


    for y in 0..screen.height
    {
        let row = y * screen.width;
        for x in 0..screen.width
        {
            let char_pos = x + row;
            
            let character_diff: bool = screen.buffer[char_pos].character != back_screen.buffer[char_pos].character; 
            let rgb_diff: bool = screen.buffer[char_pos].r != back_screen.buffer[char_pos].r
                || screen.buffer[char_pos].g != back_screen.buffer[char_pos].g
                || screen.buffer[char_pos].b != back_screen.buffer[char_pos].b;

            if character_diff || rgb_diff
            {
                screen.buffer[char_pos].character = back_screen.buffer[char_pos].character; 
                screen.buffer[char_pos].r = back_screen.buffer[char_pos].r; 
                screen.buffer[char_pos].g = back_screen.buffer[char_pos].g; 
                screen.buffer[char_pos].b = back_screen.buffer[char_pos].b; 

                let char_val: char = screen.buffer[char_pos].character;

                begin_esc();
                set_cursor_to_esc(x as i16, y as i16);
                set_foreground_color_esc(screen.buffer[char_pos].r, screen.buffer[char_pos].g, screen.buffer[char_pos].b);
                end_esc();
                
                
                if char_val == '\0'
                {
                    print!(" ");
                }
                else
                {
                    print!("{}", char_val);
                }
            }
        }
    }
    begin_esc();
    restore_cursor_position_esc();
    end_esc();
}


fn write_char_at_pos(screen: &mut Terminal_Screen, character: char, r: u8, g: u8, b: u8, x: i16, y: i16)
{
    if x < 0 || y < 0
    {
        return;
    }

    if x as usize >= screen.width || y as usize >= screen.height
    {
        return;
    }
    screen.buffer[x as usize + y as usize * screen.width].character = character;
    screen.buffer[x as usize + y as usize * screen.width].r = r;
    screen.buffer[x as usize + y as usize * screen.width].g = g;
    screen.buffer[x as usize + y as usize * screen.width].b = b;
}


fn write_string_at_pos(screen: &mut Terminal_Screen, str: &[char], r: u8, g: u8, b: u8, x: i16, y: i16)
{
    if x < 0 || y < 0
    {
        return;
    }
    if x as usize >= screen.width || y as usize >= screen.height
    {
        return;
    }

    let start_pos: usize = x as usize + y as usize * screen.width;


    let normalized_len: usize;
    if (str.len() + x as usize) < screen.width
    {
        normalized_len = str.len();
    }
    else
    {
        normalized_len = screen.width - 1 - x as usize;
    }

    for i in 0..normalized_len
    {
        screen.buffer[start_pos + i].character = str[i] as char;
        screen.buffer[start_pos + i].r = r;
        screen.buffer[start_pos + i].g = g;
        screen.buffer[start_pos + i].b = b;
    }
}

//TODO(Johan): only rerender differences in terminal screen to avoid flickering and to enable RGB in the terminal 
//TODO(Johan): maybe switch to ReadConsoleInputEx to avoid a blocking input
fn get_console_input() -> Input
{
    let mut input: Input = unsafe {std::mem::zeroed()};

    let mut buf: INPUT_RECORD = unsafe {std::mem::zeroed()};
    let num_char_to_read: DWORD = 1;
    let mut num_chars_read: DWORD = 0;
    if unsafe {ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &mut buf as PINPUT_RECORD, num_char_to_read, &mut num_chars_read) == FALSE}
    {
        exit(-1)
    }

    match buf.EventType
    {
        FOCUS_EVENT => {},
        KEY_EVENT =>
        {
            let key_event = unsafe {buf.event.KeyEvent};
            let key_code = key_event.wVirtualKeyCode;
            if key_event.bKeyDown == TRUE
            {
                input.unicode_char = unsafe {buf.event.KeyEvent.u_char.UnicodeChar};
                input.key_val = key_code;
                if key_event.dwControlKeyState == CAPSLOCK_ON {input.CAPSLOCK_ON = true}
                if key_event.dwControlKeyState == ENHANCED_KEY {input.ENHANCED_KEY = true}
                if key_event.dwControlKeyState == LEFT_ALT_PRESSED {input.LEFT_ALT_PRESSED = true}
                if key_event.dwControlKeyState == LEFT_CTRL_PRESSED {input.LEFT_CTRL_PRESSED = true}
                if key_event.dwControlKeyState == NUMLOCK_ON {input.NUMLOCK_ON = true}
                if key_event.dwControlKeyState == RIGHT_ALT_PRESSED {input.RIGHT_ALT_PRESSED = true}
                if key_event.dwControlKeyState == RIGHT_CTRL_PRESSED {input.RIGHT_CTRL_PRESSED = true}
                if key_event.dwControlKeyState == SCROLLLOCK_ON {input.SCROLLLOCK_ON = true}
                if key_event.dwControlKeyState == SHIFT_PRESSED {input.SHIFT_PRESSED = true}
            }
        },
        MENU_EVENT => {},
        MOUSE_EVENT => {},
        WINDOW_BUFFER_SIZE_EVENT => {},
        _ => {panic!("unreachable")}
    }
    return input;
}

struct Input
{
    unicode_char: u16,
    key_val: u16,
    CAPSLOCK_ON: bool,
    ENHANCED_KEY: bool,
    LEFT_ALT_PRESSED: bool,
    LEFT_CTRL_PRESSED: bool,
    NUMLOCK_ON: bool,
    RIGHT_ALT_PRESSED: bool,
    RIGHT_CTRL_PRESSED: bool,
    SCROLLLOCK_ON: bool,
    SHIFT_PRESSED: bool,
}


static mut back_buffer: Terminal_Screen = Terminal_Screen { buffer: vec![], width: 0, height: 0 };
static mut front_buffer: Terminal_Screen = Terminal_Screen { buffer: vec![], width: 0, height: 0 };


static mut terminal_x: SHORT = 0;
static mut terminal_y: SHORT = 0;
static mut terminal_w: SHORT = 0;
static mut terminal_h: SHORT = 0;
fn get_terminal_screen() -> &'static mut Terminal_Screen
{
    unsafe
    {
        let stdout: HANDLE = GetStdHandle(STD_OUTPUT_HANDLE);
        let mut screen_buf_info: CONSOLE_SCREEN_BUFFER_INFO = std::mem::zeroed();
        if GetConsoleScreenBufferInfo(stdout, &mut screen_buf_info as PCONSOLE_SCREEN_BUFFER_INFO) == FALSE
        {
            eprintln!("ERROR: GetConsoleScreenBufferInfo {}", GetLastError());
            exit(-1);
        }

        let new_x = screen_buf_info.srWindow.Left;
        let new_y = screen_buf_info.srWindow.Top;
        let new_w = screen_buf_info.srWindow.Right - screen_buf_info.srWindow.Left;
        let new_h = screen_buf_info.srWindow.Bottom - screen_buf_info.srWindow.Top;


        let screen_resized: bool = terminal_x != new_x || terminal_y != new_y || terminal_w != new_w || terminal_h != new_h;
        if screen_resized 
        {
            begin_esc();
            clear_screen_esc();
            end_esc();
            // resize the buffer as the window has changed size
            terminal_x = new_x;
            terminal_y = new_y;
            terminal_w = new_w;
            terminal_h = new_h;

            let buffer_length = (terminal_w * terminal_h) as usize;

            let width = terminal_w as usize;
            let height = terminal_h as usize;
            back_buffer.width = width;
            back_buffer.height = height;

            back_buffer.buffer.reserve(buffer_length);
            for _ in 0..buffer_length
            {
                back_buffer.buffer.insert(0, Screen_Element { character: '\0', r: 0, g: 0, b: 0 });
            }


            front_buffer.width = width;
            front_buffer.height = height;

            front_buffer.buffer.reserve(buffer_length);
            for _ in 0..buffer_length
            {
                front_buffer.buffer.insert(0, Screen_Element { character: '\0', r: 0, g: 0, b: 0 });
            }

            // println!("terminal terminal_x = {}, terminal_y = {}, terminal_w = {}, terminal_h = {}", terminal_x, terminal_y, terminal_w, terminal_h);
        }
        return &mut back_buffer; 
    }
}

fn set_foreground_color_esc(r: u8, g: u8, b: u8)
{
    assert!(unsafe {debug_escape});
    print!("\x1b[38;2;{};{};{}m", r, g, b);
}

fn set_background_color_esc(r: u8, g: u8, b: u8)
{
    assert!(unsafe {debug_escape});
    print!("\x1b[48;2;{};{};{}m", r, g, b);
}

fn set_default_color_esc()
{
    assert!(unsafe {debug_escape});
    print!("\x1b[0m");   
}






fn main()
{
    let mut old_stdin_mode: DWORD = 0;
    let mut old_stdout_mode: DWORD = 0;
    let stdin: HANDLE;
    let stdout: HANDLE;

    unsafe
    {
        stdin = GetStdHandle(STD_INPUT_HANDLE);
        if stdin == INVALID_HANDLE_VALUE
        {
            exit(420);
        }

        stdout = GetStdHandle(STD_OUTPUT_HANDLE);
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
    }

    let bnf_src = include_str!("bnf.txt");


    let mut table: [u8; 10000] = [0; 10000];
    let table_size_diff: u32 = unsafe {write_parse_table_from_bnf(&mut table as *mut u8 as *mut c_void, 10000, bnf_src.as_ptr() as *const i8)};
    if table_size_diff != 0
    {
        println!("failed to read table {}", table_size_diff);
        return;
    }


    let mut predefined_functions: HashMap<String_View, fn(f64) -> f64> = HashMap::new();

    predefined_functions.insert(str_view_from_str("sin"), f64::sin);
    predefined_functions.insert(str_view_from_str("cos"), f64::cos);
    predefined_functions.insert(str_view_from_str("tan"), f64::tan);

    predefined_functions.insert(str_view_from_str("asin"), f64::asin);
    predefined_functions.insert(str_view_from_str("acos"), f64::acos);
    predefined_functions.insert(str_view_from_str("atan"), f64::atan);

    predefined_functions.insert(str_view_from_str("sqrt"), f64::sqrt);
    predefined_functions.insert(str_view_from_str("cbrt"), f64::cbrt);

    predefined_functions.insert(str_view_from_str("exp"), f64::exp);

    predefined_functions.insert(str_view_from_str("ln"), f64::ln);
    predefined_functions.insert(str_view_from_str("log2"), f64::log2);
    predefined_functions.insert(str_view_from_str("log10"), f64::log10);

    predefined_functions.insert(str_view_from_str("floor"), f64::floor);
    predefined_functions.insert(str_view_from_str("ceil"), f64::ceil);

    predefined_functions.insert(str_view_from_str("abs"), f64::abs);


    const length: usize = 32;
    const buffer_count: usize = 5;
    let mut input_buffers: Vec<[char; length]> = Vec::new();
    let mut output_buffers: Vec<[char; length]> = Vec::new();
    let mut token_buffers: Vec<Vec<ParseToken>> = Vec::new();
    input_buffers.reserve_exact(buffer_count);
    output_buffers.reserve_exact(buffer_count);
    token_buffers.reserve_exact(buffer_count);
    for _ in 0..buffer_count
    {
        input_buffers.push(['\0'; length]);
        output_buffers.push(['\0'; length]);
        token_buffers.push(Vec::new());
    }


    let mut expr: *mut Expr = 0 as *mut Expr;
    let mut msg: [u8; 1000] = [0; 1000];
    let mut map: HashMap<String_View, Symbol> = HashMap::new();

    
    begin_esc();
    // change to alternate buffer
    print!("\x1b[?1049h");
    set_cursor_to_esc(0, 0);
    end_esc();
    unsafe
    {
        loop
        {
            let mut screen = get_terminal_screen();
            let inputs: Input = get_console_input();

            if inputs.key_val == 'Q' as u16
            {
                if inputs.LEFT_ALT_PRESSED
                {
                    break;
                }
            }
            if inputs.key_val == 'C' as u16
            {
                if inputs.LEFT_CTRL_PRESSED
                {
                    break;
                }
            }


            fn write_string_to_buffer(buffer: &mut [char], string: &String)
            {
                for i in 0..buffer.len()
                {
                    buffer[i] = '\0'
                }
                let mut j = 0;
                for character in string.chars()
                {
                    if j == buffer.len()
                    {
                        break;
                    }
                    buffer[j] = character;
                    j += 1;
                }
            }
            

            // ui
            {
                let layout: Ui_Layout = Ui_Layout { 
                    x: 0, 
                    y: 0, 
                    w: screen.width, 
                    h: screen.height, 
                    flow: Flow::UPPER_LEFT_FLOW_DOWN,
                };

                let mut should_reparse: bool = false;

                push_layout(layout);
                for i in 0..buffer_count
                {
                    let xy = get_xy_with_current_layout(length, INPUT_STR_HEIGHT);

                    let layout2: Ui_Layout = Ui_Layout { 
                        x: xy.0, 
                        y: xy.1, 
                        w: 1 + 2 * length, 
                        h: INPUT_STR_HEIGHT, 
                        flow: Flow::UPPER_LEFT_FLOW_RIGHT,
                    };
                    push_layout(layout2);
                    match input_string_box(&mut screen, str_as_usize("my_text_box") + i, input_buffers[i].as_mut_slice(), length, &inputs)
                    {
                        Input_Box_Actions::None => {},
                        Input_Box_Actions::LEAVE_WITH_ESCAPE | Input_Box_Actions::LEAVE_WITH_ENTER =>
                        {
                            should_reparse = true;
                        }
                    }
                    output_string_box(&mut screen, str_as_usize("my_output_string") + i, output_buffers[i].as_slice(), length);
                    pop_layout();
                }
                pop_layout();
                if should_reparse
                {
                    map.clear();
                    'outer: for i in 0..buffer_count
                    {
                        let str_len = str_buffer_len(&input_buffers[i], length);
    
                        token_buffers[i].clear();
                        if let Err(x) = string_to_tokens(&input_buffers[i], str_len, &mut token_buffers[i])
                        {
                            write_string_to_buffer(&mut output_buffers[i], &x);
                            continue 'outer;
                        }


                        let success: bool = parse_bin(token_buffers[i].as_mut_ptr(), token_buffers[i].len() as u32,
                            table.as_mut_ptr(), 0, &mut expr, msg.as_mut_ptr() as *mut i8, 1000);


                        if success
                        {
                            graphviz_from_syntax_tree(b"./input.dot\0".as_ptr() as *const i8, expr);

                            // print_tree(expr);


                            if get_expr_token_type(expr) != LR_Type::ExpraltS ||
                                (*expr).expr_count == 0
                            {
                                continue 'outer;
                            }

                            let decl_expr = get_expr_in_array(expr, 0);
                            let token_type = get_expr_token_type(decl_expr);

                            if token_type == LR_Type::ExprVarDecl
                            {
                                let str: String = add_var_decl(decl_expr, &mut map);
                                write_string_to_buffer(&mut output_buffers[i], &str);
                            }
                            else if LR_Type::ExprFuncDecl == token_type
                            {

                                let (Ok(x) | Err(x)) = add_func_decl(decl_expr, &mut map, &predefined_functions);
                                write_string_to_buffer(&mut output_buffers[i], &x);
                            }
                            else if LR_Type::ExprE == token_type
                            {
                                match eval_tree(expr, &mut map, None, &predefined_functions)
                                {
                                    Ok(x) => write_string_to_buffer(&mut output_buffers[i], &format!(" = {}", x)),
                                    Err(x) => write_string_to_buffer(&mut output_buffers[i], &format!("{}", x)),
                                }
                            }
                            else
                            {
                                panic!("unreachable");
                            }
                        }
                        else
                        {
                            for j in 0..length
                            {
                                output_buffers[i][j] = msg[j] as char;
                            }
                        }
                    }
                }
            }

            let bottom_y = screen.height as i16 - 1;
            let format_string: Vec<char> = format!("pos=[{} {}] unicode_u16={}, inside={:?}, active={:?}",
                cursor.x,
                cursor.y,
                inputs.unicode_char,
                hot.item,
                active).chars().collect::<Vec<char>>();

            write_string_at_pos(&mut screen, format_string.as_slice(), 255, 255, 255, 0, bottom_y);

            if active.stack_count == 0
            {
                if inputs.key_val == VK_UP
                {
                    begin_esc();
                    move_cursor_esc(0, -1);
                    end_esc();
                }
                else if inputs.key_val == VK_LEFT
                {
                    begin_esc();
                    move_cursor_esc(-1, 0);
                    end_esc();
                }
                else if inputs.key_val == VK_DOWN
                {
                    begin_esc();
                    move_cursor_esc(0, 1);
                    end_esc();
                }
                else if inputs.key_val == VK_RIGHT
                {
                    begin_esc();
                    move_cursor_esc(1, 0);
                    end_esc();
                }
            }

            present_back_terminal_buffer();
            io::stdout().flush().unwrap();
        }
        revert_console(stdout, stdin, old_stdout_mode, old_stdin_mode);
        exit(0);
    }
}