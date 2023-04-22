#![allow(unused)]

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]

pub type HANDLE = *mut std::ffi::c_void;
pub type LPVOID = *mut std::ffi::c_void;
pub type DWORD = std::ffi::c_ulong;
pub type LPDWORD = *mut DWORD;
pub type BOOL = std::ffi::c_int;
pub type WORD = std::ffi::c_ushort;

pub type WCHAR = u16;
pub type CHAR = std::ffi::c_char;
pub type SHORT = std::ffi::c_short;
pub type UINT = std::ffi::c_uint;
pub const TRUE: BOOL = 1;
pub const FALSE: BOOL = 0;

pub const STD_INPUT_HANDLE: DWORD = DWORD::MAX - 10 + 1; 
pub const STD_OUTPUT_HANDLE: DWORD = DWORD::MAX - 11 + 1;
pub const STD_ERROR_HANDLE: DWORD = DWORD::MAX - 12 + 1;
pub const INVALID_HANDLE_VALUE: HANDLE = unsafe {std::mem::transmute::<i64, HANDLE>(-1)};

pub const FOCUS_EVENT: WORD = 0x0010;
pub const KEY_EVENT: WORD = 0x0001;
pub const MENU_EVENT: WORD = 0x0008;
pub const MOUSE_EVENT: WORD = 0x0002;
pub const WINDOW_BUFFER_SIZE_EVENT: WORD = 0x0004;


pub const CAPSLOCK_ON: DWORD = 0x0080;
pub const ENHANCED_KEY: DWORD = 0x0100;
pub const LEFT_ALT_PRESSED: DWORD = 0x0002;
pub const LEFT_CTRL_PRESSED: DWORD = 0x0008;
pub const NUMLOCK_ON: DWORD = 0x0020;
pub const RIGHT_ALT_PRESSED: DWORD = 0x0001;
pub const RIGHT_CTRL_PRESSED: DWORD = 0x0004;
pub const SCROLLLOCK_ON: DWORD = 0x0040;
pub const SHIFT_PRESSED: DWORD = 0x0010;


#[repr(C)]
#[derive(Copy, Clone)]
pub struct FOCUS_EVENT_RECORD
{
    pub bSetFocus: BOOL,
}
#[repr(C)]
#[derive(Copy, Clone)]
pub union uChar
{
    pub UnicodeChar: WCHAR,
    pub AsciiChar: CHAR,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct KEY_EVENT_RECORD
{
    pub bKeyDown: BOOL,
    pub wRepeatCount: WORD,
    pub wVirtualKeyCode: WORD,
    pub wVirtualScanCode: WORD,
    pub u_char: uChar,
    pub dwControlKeyState: DWORD
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct MOUSE_EVENT_RECORD 
{
    pub dwMousePosition: COORD,
    pub dwButtonState: DWORD,
    pub dwControlKeyState: DWORD,
    pub dwEventFlags: DWORD,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct COORD
{
    pub X: SHORT,
    pub Y: SHORT,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct WINDOW_BUFFER_SIZE_RECORD
{
    pub dwSize: COORD,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct MENU_EVENT_RECORD
{
    pub dwCommandId: UINT,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub union Event 
{
    pub KeyEvent: KEY_EVENT_RECORD,
    pub MouseEvent: MOUSE_EVENT_RECORD,
    pub WindowBufferSizeEvent: WINDOW_BUFFER_SIZE_RECORD,
    pub MenuEvent: MENU_EVENT_RECORD,
    pub FocusEvent: FOCUS_EVENT_RECORD,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct INPUT_RECORD 
{
    pub EventType: WORD,
    pub event: Event,

}

pub type PINPUT_RECORD = *mut INPUT_RECORD;

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct SMALL_RECT
{
    pub Left: SHORT,
    pub Top: SHORT,
    pub Right: SHORT,
    pub Bottom: SHORT,
}


#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct CONSOLE_SCREEN_BUFFER_INFO
{
    pub dwSize: COORD,
    pub dwCursorPosition: COORD,
    pub wAttributes: WORD,
    pub srWindow: SMALL_RECT,
    pub dwMaximumWindowSize: COORD,
}

pub type PCONSOLE_SCREEN_BUFFER_INFO = *mut CONSOLE_SCREEN_BUFFER_INFO; 
extern "stdcall"
{
    pub fn ReadConsoleInputW(hConsoleInput: HANDLE, lpBuffer: PINPUT_RECORD, nLength: DWORD, lpNumberOfEventsRead: LPDWORD) -> BOOL;
    pub fn SetConsoleMode(hConsoleHandle: HANDLE, dwMode: DWORD) -> BOOL;
    pub fn GetStdHandle(nStdHandle: DWORD) -> HANDLE;
    pub fn GetConsoleScreenBufferInfo(hConsoleOutput: HANDLE, lpConsoleScreenBufferInfo: PCONSOLE_SCREEN_BUFFER_INFO) -> BOOL;
    pub fn GetLastError() -> DWORD;
}


