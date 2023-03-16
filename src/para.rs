#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]

use core::ffi::c_void;
use std::mem::transmute;
extern "C"
{
    fn LoadLibraryA(lpLibFileName: *const u8) -> *const c_void;
    fn GetProcAddress(hModule: *const c_void, lpProcName: *const u8) -> *const c_void;
}

struct Lexer;
type CreateLexerFromBnfT = fn(src: *const u8) -> *mut Lexer; 

fn stub(src: *const u8) -> *mut Lexer
{
    return 0 as *mut Lexer;
}

static mut create_lexer_from_bnf: CreateLexerFromBnfT = stub;

fn main()
{
    unsafe
    {
        let ptg_dll = LoadLibraryA(b"./lib/ptg.dll\0".as_ptr());
        
        if ptg_dll == 0 as *const c_void
        {
            println!("Failed to load ptg_dll");
            return;  
        }
        {
            let addr = GetProcAddress(ptg_dll, b"create_lexer_from_bnf\0".as_ptr());
            if addr == 0 as *const c_void
            {
                println!("Failed to load create_lexer_from_bnf");
                return;
            }
            create_lexer_from_bnf = transmute::<*const c_void, CreateLexerFromBnfT>(addr);
        }

    }

    let lexer = unsafe{create_lexer_from_bnf(b"test".as_ptr())};

    println!("hello world");
}