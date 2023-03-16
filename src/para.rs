#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]


use core::ffi::c_void;
use std::mem::transmute;
use std::fs::File;
use std::io::BufReader;
use std::io::Read;
extern "C"
{
    fn LoadLibraryA(lpLibFileName: *const u8) -> *const c_void;
    fn GetProcAddress(hModule: *const c_void, lpProcName: *const u8) -> *const c_void;
}

struct ParseToken
{
    token_type: i64,
    data: *const u8,
    length: u32,
}

struct Expr
{
    token: ParseToken,
    expr_count: u32,
    exprs: [*mut Expr],
}

type WriteParseTableFromBnfT = fn(buffer: *mut c_void, buffer_size: u32, src: *const u8) -> u32;
type ParseBinT = fn(token_list: *mut ParseToken, token_count: u32, table: *mut u8, flags: u32, opt_tree_out: *mut*mut Expr) -> bool;


static mut write_parse_table_from_bnf: WriteParseTableFromBnfT = |_, _, _,| 0;
static mut parse_bin: ParseBinT = |_, _, _, _, _,| false;


fn main()
{
    let mut bnf_src: String;
    {
        let file = File::open("./src/bnf.txt").unwrap();
        let mut buf_reader = BufReader::new(file);
        bnf_src = String::new();
        buf_reader.read_to_string(&mut bnf_src).unwrap();
    }

    unsafe
    {
        let ptg_dll = LoadLibraryA(b"./lib/ptg.dll\0".as_ptr());
        
        if ptg_dll == 0 as *const c_void
        {
            println!("Failed to load ptg_dll");
            return;  
        }
        {
            let addr = GetProcAddress(ptg_dll, b"write_parse_table_from_bnf\0".as_ptr());
            if addr == 0 as *const c_void
            {
                println!("Failed to load write_parse_table_from_bnf");
                return;
            }
            write_parse_table_from_bnf = transmute::<*const c_void, WriteParseTableFromBnfT>(addr);
        }
        {
            let addr = GetProcAddress(ptg_dll, b"parse_bin\0".as_ptr());
            if addr == 0 as *const c_void
            {
                println!("Failed to load parse_bin");
                return;
            }
            parse_bin = transmute::<*const c_void, ParseBinT>(addr);
        }

    }
    let mut table: [u8; 10000] = [0; 10000];
    let table_size: u32 = unsafe {write_parse_table_from_bnf(&mut table as *mut u8 as *mut c_void, 5000, bnf_src.as_bytes().as_ptr())};
    if table_size > 10000
    {
        println!("failed to read table {}", table_size);
        return;
    }
    let mut expr: *mut Expr;
    let success: bool = unsafe {parse_bin()}



    println!("hello world");
}