#![allow(unused)]

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]

// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
// https://learn.microsoft.com/en-us/windows/console/getconsolemode


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



pub const VK_LBUTTON: WORD = 0x01; // Left mouse button
pub const VK_RBUTTON: WORD = 0x02; // Right mouse button
pub const VK_CANCEL: WORD = 0x03; // Control-break processing
pub const VK_MBUTTON: WORD = 0x04; // Middle mouse button (three-button mouse)
pub const VK_XBUTTON1: WORD = 0x05; // X1 mouse button
pub const VK_XBUTTON2: WORD = 0x06; // X2 mouse button
//- 	0x07 	Undefined
pub const VK_BACK: WORD = 0x08; // BACKSPACE key
pub const VK_TAB: WORD = 0x09; // TAB key
//- 	0x0A-0B 	Reserved
pub const VK_CLEAR: WORD = 0x0C; // CLEAR key
pub const VK_RETURN: WORD = 0x0D; // ENTER key
//- 	0x0E-0F 	Undefined
pub const VK_SHIFT: WORD = 0x10; // SHIFT key
pub const VK_CONTROL: WORD = 0x11; // CTRL key
pub const VK_MENU: WORD = 0x12; // ALT key
pub const VK_PAUSE: WORD = 0x13; // PAUSE key
pub const VK_CAPITAL: WORD = 0x14; // CAPS LOCK key
pub const VK_KANA: WORD = 0x15; // IME Kana mode
pub const VK_HANGUEL: WORD = 0x15; // IME Hanguel mode (maintained for compatibility; use VK_HANGUL)
pub const VK_HANGUL: WORD = 0x15; // IME Hangul mode
pub const VK_IME_ON: WORD = 0x16; // IME On
pub const VK_JUNJA: WORD = 0x17; // IME Junja mode
pub const VK_FINAL: WORD = 0x18; // IME final mode
pub const VK_HANJA: WORD = 0x19; // IME Hanja mode
pub const VK_KANJI: WORD = 0x19; // IME Kanji mode
pub const VK_IME_OFF: WORD = 0x1A; // IME Off
pub const VK_ESCAPE: WORD = 0x1B; // ESC key
pub const VK_CONVERT: WORD = 0x1C; // IME convert
pub const VK_NONCONVERT: WORD = 0x1D; // IME nonconvert
pub const VK_ACCEPT: WORD = 0x1E; // IME accept
pub const VK_MODECHANGE: WORD = 0x1F; // IME mode change request
pub const VK_SPACE: WORD = 0x20; // SPACEBAR
pub const VK_PRIOR: WORD = 0x21; // PAGE UP key
pub const VK_NEXT: WORD = 0x22; // PAGE DOWN key
pub const VK_END: WORD = 0x23; // END key
pub const VK_HOME: WORD = 0x24; // HOME key
pub const VK_LEFT: WORD = 0x25; // LEFT ARROW key
pub const VK_UP: WORD = 0x26; // UP ARROW key
pub const VK_RIGHT: WORD = 0x27; // RIGHT ARROW key
pub const VK_DOWN: WORD = 0x28; // DOWN ARROW key
pub const VK_SELECT: WORD = 0x29; // SELECT key
pub const VK_PRINT: WORD = 0x2A; // PRINT key
pub const VK_EXECUTE: WORD = 0x2B; // EXECUTE key
pub const VK_SNAPSHOT: WORD = 0x2C; // PRINT SCREEN key
pub const VK_INSERT: WORD = 0x2D; // INS key
pub const VK_DELETE: WORD = 0x2E; // DEL key
pub const VK_HELP: WORD = 0x2F; // HELP key
//	0x30 	0 key
//	0x31 	1 key
//	0x32 	2 key
//	0x33 	3 key
//	0x34 	4 key
//	0x35 	5 key
//	0x36 	6 key
//	0x37 	7 key
//	0x38 	8 key
//	0x39 	9 key
//- 	0x3A-40 	Undefined
//	0x41 	A key
//	0x42 	B key
//	0x43 	C key
//	0x44 	D key
//	0x45 	E key
//	0x46 	F key
//	0x47 	G key
//	0x48 	H key
//	0x49 	I key
//	0x4A 	J key
//	0x4B 	K key
//	0x4C 	L key
//	0x4D 	M key
//	0x4E 	N key
//	0x4F 	O key
//	0x50 	P key
//	0x51 	Q key
//	0x52 	R key
//	0x53 	S key
//	0x54 	T key
//	0x55 	U key
//	0x56 	V key
//	0x57 	W key
//	0x58 	X key
//	0x59 	Y key
//	0x5A 	Z key
pub const VK_LWIN: WORD = 0x5B; // Left Windows key (Natural keyboard)
pub const VK_RWIN: WORD = 0x5C; // Right Windows key (Natural keyboard)
pub const VK_APPS: WORD = 0x5D; // Applications key (Natural keyboard)
//- 	0x5E 	Reserved
pub const VK_SLEEP: WORD = 0x5F; // Computer Sleep key
pub const VK_NUMPAD0: WORD = 0x60; // Numeric keypad 0 key
pub const VK_NUMPAD1: WORD = 0x61; // Numeric keypad 1 key
pub const VK_NUMPAD2: WORD = 0x62; // Numeric keypad 2 key
pub const VK_NUMPAD3: WORD = 0x63; // Numeric keypad 3 key
pub const VK_NUMPAD4: WORD = 0x64; // Numeric keypad 4 key
pub const VK_NUMPAD5: WORD = 0x65; // Numeric keypad 5 key
pub const VK_NUMPAD6: WORD = 0x66; // Numeric keypad 6 key
pub const VK_NUMPAD7: WORD = 0x67; // Numeric keypad 7 key
pub const VK_NUMPAD8: WORD = 0x68; // Numeric keypad 8 key
pub const VK_NUMPAD9: WORD = 0x69; // Numeric keypad 9 key
pub const VK_MULTIPLY: WORD = 0x6A; // Multiply key
pub const VK_ADD: WORD = 0x6B; // Add key
pub const VK_SEPARATOR: WORD = 0x6C; // Separator key
pub const VK_SUBTRACT: WORD = 0x6D; // Subtract key
pub const VK_DECIMAL: WORD = 0x6E; // Decimal key
pub const VK_DIVIDE: WORD = 0x6F; // Divide key
pub const VK_F1: WORD = 0x70; // F1 key
pub const VK_F2: WORD = 0x71; // F2 key
pub const VK_F3: WORD = 0x72; // F3 key
pub const VK_F4: WORD = 0x73; // F4 key
pub const VK_F5: WORD = 0x74; // F5 key
pub const VK_F6: WORD = 0x75; // F6 key
pub const VK_F7: WORD = 0x76; // F7 key
pub const VK_F8: WORD = 0x77; // F8 key
pub const VK_F9: WORD = 0x78; // F9 key
pub const VK_F10: WORD = 0x79; // F10 key
pub const VK_F11: WORD = 0x7A; // F11 key
pub const VK_F12: WORD = 0x7B; // F12 key
pub const VK_F13: WORD = 0x7C; // F13 key
pub const VK_F14: WORD = 0x7D; // F14 key
pub const VK_F15: WORD = 0x7E; // F15 key
pub const VK_F16: WORD = 0x7F; // F16 key
pub const VK_F17: WORD = 0x80; // F17 key
pub const VK_F18: WORD = 0x81; // F18 key
pub const VK_F19: WORD = 0x82; // F19 key
pub const VK_F20: WORD = 0x83; // F20 key
pub const VK_F21: WORD = 0x84; // F21 key
pub const VK_F22: WORD = 0x85; // F22 key
pub const VK_F23: WORD = 0x86; // F23 key
pub const VK_F24: WORD = 0x87; // F24 key
//- 	0x88-8F 	Unassigned
pub const VK_NUMLOCK: WORD = 0x90; // NUM LOCK key
pub const VK_SCROLL: WORD = 0x91; // SCROLL LOCK key
//	0x92-96 	OEM specific
//- 	0x97-9F 	Unassigned
pub const VK_LSHIFT: WORD = 0xA0; // Left SHIFT key
pub const VK_RSHIFT: WORD = 0xA1; // Right SHIFT key
pub const VK_LCONTROL: WORD = 0xA2; // Left CONTROL key
pub const VK_RCONTROL: WORD = 0xA3; // Right CONTROL key
pub const VK_LMENU: WORD = 0xA4; // Left ALT key
pub const VK_RMENU: WORD = 0xA5; // Right ALT key
pub const VK_BROWSER_BACK: WORD = 0xA6; // Browser Back key
pub const VK_BROWSER_FORWARD: WORD = 0xA7; // Browser Forward key
pub const VK_BROWSER_REFRESH: WORD = 0xA8; // Browser Refresh key
pub const VK_BROWSER_STOP: WORD = 0xA9; // Browser Stop key
pub const VK_BROWSER_SEARCH: WORD = 0xAA; // Browser Search key
pub const VK_BROWSER_FAVORITES: WORD = 0xAB; // Browser Favorites key
pub const VK_BROWSER_HOME: WORD = 0xAC; // Browser Start and Home key
pub const VK_VOLUME_MUTE: WORD = 0xAD; // Volume Mute key
pub const VK_VOLUME_DOWN: WORD = 0xAE; // Volume Down key
pub const VK_VOLUME_UP: WORD = 0xAF; // Volume Up key
pub const VK_MEDIA_NEXT_TRACK: WORD = 0xB0; // Next Track key
pub const VK_MEDIA_PREV_TRACK: WORD = 0xB1; // Previous Track key
pub const VK_MEDIA_STOP: WORD = 0xB2; // Stop Media key
pub const VK_MEDIA_PLAY_PAUSE: WORD = 0xB3; // Play/Pause Media key
pub const VK_LAUNCH_MAIL: WORD = 0xB4; // Start Mail key
pub const VK_LAUNCH_MEDIA_SELECT: WORD = 0xB5; // Select Media key
pub const VK_LAUNCH_APP1: WORD = 0xB6; // Start Application 1 key
pub const VK_LAUNCH_APP2: WORD = 0xB7; // Start Application 2 key
//- 	0xB8-B9 	Reserved
pub const VK_OEM_1: WORD = 0xBA; // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key
pub const VK_OEM_PLUS: WORD = 0xBB; // For any country/region, the '+' key
pub const VK_OEM_COMMA: WORD = 0xBC; // For any country/region, the ',' key
pub const VK_OEM_MINUS: WORD = 0xBD; // For any country/region, the '-' key
pub const VK_OEM_PERIOD: WORD = 0xBE; // For any country/region, the '.' key
pub const VK_OEM_2: WORD = 0xBF; // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
pub const VK_OEM_3: WORD = 0xC0; // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
//- 	0xC1-D7 	Reserved
//- 	0xD8-DA 	Unassigned
pub const VK_OEM_4: WORD = 0xDB; // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
pub const VK_OEM_5: WORD = 0xDC; // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
pub const VK_OEM_6: WORD = 0xDD; // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
pub const VK_OEM_7: WORD = 0xDE; // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
pub const VK_OEM_8: WORD = 0xDF; // Used for miscellaneous characters; it can vary by keyboard.
//- 	0xE0 	Reserved
//	0xE1 	OEM specific
pub const VK_OEM_102: WORD = 0xE2; // The <> keys on the US standard keyboard, or the \\| key on the non-US 102-key keyboard
//	0xE3-E4 	OEM specific
pub const VK_PROCESSKEY: WORD = 0xE5; // IME PROCESS key
//	0xE6 	OEM specific
pub const VK_PACKET: WORD = 0xE7; // Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP
//- 	0xE8 	Unassigned
//	0xE9-F5 	OEM specific
pub const VK_ATTN: WORD = 0xF6; // Attn key
pub const VK_CRSEL: WORD = 0xF7; // CrSel key
pub const VK_EXSEL: WORD = 0xF8; // ExSel key
pub const VK_EREOF: WORD = 0xF9; // Erase EOF key
pub const VK_PLAY: WORD = 0xFA; // Play key
pub const VK_ZOOM: WORD = 0xFB; // Zoom key
pub const VK_NONAME: WORD = 0xFC; // Reserved
pub const VK_PA1: WORD = 0xFD; // PA1 key
pub const VK_OEM_CLEAR: WORD = 0xFE; // Clear key



pub const ENABLE_ECHO_INPUT: DWORD = 0x0004; //  	Characters read by the ReadFile or ReadConsole function are written to the active screen buffer as they are typed into the console. This mode can be used only if the ENABLE_LINE_INPUT mode is also enabled.
pub const ENABLE_INSERT_MODE: DWORD = 0x0020; //  	When enabled, text entered in a console window will be inserted at the current cursor location and all text following that location will not be overwritten. When disabled, all following text will be overwritten.
pub const ENABLE_LINE_INPUT: DWORD = 0x0002; //  	The ReadFile or ReadConsole function returns only when a carriage return character is read. If this mode is disabled, the functions return when one or more characters are available.
pub const ENABLE_MOUSE_INPUT: DWORD = 0x0010; //  	If the mouse pointer is within the borders of the console window and the window has the keyboard focus, mouse events generated by mouse movement and button presses are placed in the input buffer. These events are discarded by ReadFile or ReadConsole, even when this mode is enabled. The ReadConsoleInput function can be used to read MOUSE_EVENT input records from the input buffer.
pub const ENABLE_PROCESSED_INPUT: DWORD = 0x0001; //  	CTRL+C is processed by the system and is not placed in the input buffer. If the input buffer is being read by ReadFile or ReadConsole, other control keys are processed by the system and are not returned in the ReadFile or ReadConsole buffer. If the ENABLE_LINE_INPUT mode is also enabled, backspace, carriage return, and line feed characters are handled by the system.
pub const ENABLE_QUICK_EDIT_MODE: DWORD = 0x0040; //  	This flag enables the user to use the mouse to select and edit text. To enable this mode, use ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS. To disable this mode, use ENABLE_EXTENDED_FLAGS without this flag.
pub const ENABLE_WINDOW_INPUT: DWORD = 0x0008; //  	User interactions that change the size of the console screen buffer are reported in the console's input buffer. Information about these events can be read from the input buffer by applications using the ReadConsoleInput function, but not by those using ReadFile or ReadConsole.
pub const ENABLE_VIRTUAL_TERMINAL_INPUT: DWORD = 0x0200; //  	Setting this flag directs the Virtual Terminal processing engine to convert user input received by the console window into Console Virtual Terminal Sequences that can be retrieved by a supporting application through ReadFile or ReadConsole functions.

//The typical usage of this flag is intended in conjunction with ENABLE_VIRTUAL_TERMINAL_PROCESSING on the output handle to connect to an application that communicates exclusively via virtual terminal sequences.

//If the hConsoleHandle parameter is a screen buffer handle, the mode can be one or more of the following values. When a screen buffer is created, both output modes are enabled by default.
//Value 	Meaning
pub const ENABLE_PROCESSED_OUTPUT: DWORD = 0x0001; //  	Characters written by the WriteFile or WriteConsole function or echoed by the ReadFile or ReadConsole function are parsed for ASCII control sequences, and the correct action is performed. Backspace, tab, bell, carriage return, and line feed characters are processed. It should be enabled when using control sequences or when ENABLE_VIRTUAL_TERMINAL_PROCESSING is set.
pub const ENABLE_WRAP_AT_EOL_OUTPUT: DWORD = 0x0002; //  	When writing with WriteFile or WriteConsole or echoing with ReadFile or ReadConsole, the cursor moves to the beginning of the next row when it reaches the end of the current row. This causes the rows displayed in the console window to scroll up automatically when the cursor advances beyond the last row in the window. It also causes the contents of the console screen buffer to scroll up (../discarding the top row of the console screen buffer) when the cursor advances beyond the last row in the console screen buffer. If this mode is disabled, the last character in the row is overwritten with any subsequent characters.
pub const ENABLE_VIRTUAL_TERMINAL_PROCESSING: DWORD = 0x0004; //  	When writing with WriteFile or WriteConsole, characters are parsed for VT100 and similar control character sequences that control cursor movement, color/font mode, and other operations that can also be performed via the existing Console APIs. For more information, see Console Virtual Terminal Sequences.
//Ensure ENABLE_PROCESSED_OUTPUT is set when using this flag.
pub const DISABLE_NEWLINE_AUTO_RETURN: DWORD = 0x0008; //  	When writing with WriteFile or WriteConsole, this adds an additional state to end-of-line wrapping that can delay the cursor move and buffer scroll operations.

//Normally when ENABLE_WRAP_AT_EOL_OUTPUT is set and text reaches the end of the line, the cursor will immediately move to the next line and the contents of the buffer will scroll up by one line. In contrast with this flag set, the cursor does not move to the next line, and the scroll operation is not performed. The written character will be printed in the final position on the line and the cursor will remain above this character as if ENABLE_WRAP_AT_EOL_OUTPUT was off, but the next printable character will be printed as if ENABLE_WRAP_AT_EOL_OUTPUT is on. No overwrite will occur. Specifically, the cursor quickly advances down to the following line, a scroll is performed if necessary, the character is printed, and the cursor advances one more position.

//The typical usage of this flag is intended in conjunction with setting ENABLE_VIRTUAL_TERMINAL_PROCESSING to better emulate a terminal emulator where writing the final character on the screen (../in the bottom right corner) without triggering an immediate scroll is the desired behavior.
pub const ENABLE_LVB_GRID_WORLDWIDE: DWORD = 0x0010; //  	The APIs for writing character attributes including WriteConsoleOutput and WriteConsoleOutputAttribute allow the usage of flags from character attributes to adjust the color of the foreground and background of text. Additionally, a range of DBCS flags was specified with the COMMON_LVB prefix. Historically, these flags only functioned in DBCS code pages for Chinese, Japanese, and Korean languages.

//With exception of the leading byte and trailing byte flags, the remaining flags describing line drawing and reverse video (../swap foreground and background colors) can be useful for other languages to emphasize portions of output.

//Setting this console mode flag will allow these attributes to be used in every code page on every language.

//It is off by default to maintain compatibility with known applications that have historically taken advantage of the console ignoring these flags on non-CJK machines to store bits in these fields for their own purposes or by accident.

//Note that using the ENABLE_VIRTUAL_TERMINAL_PROCESSING mode can result in LVB grid and reverse video flags being set while this flag is still off if the attached application requests underlining or inverse video via Console Virtual Terminal Sequences.


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
    pub fn GetConsoleMode(hConsoleHandle: HANDLE, lpMode: LPDWORD) -> BOOL;
    pub fn GetStdHandle(nStdHandle: DWORD) -> HANDLE;
    pub fn GetConsoleScreenBufferInfo(hConsoleOutput: HANDLE, lpConsoleScreenBufferInfo: PCONSOLE_SCREEN_BUFFER_INFO) -> BOOL;
    pub fn GetLastError() -> DWORD;
}


