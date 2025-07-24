#include "common.h"


// https://learn.microsoft.com/en-us/typography/opentype/spec/otff
//  32-bit signed fixed-point number (16.16)
typedef s32 Fixed; 
// int16 that describes a quantity in font design units.
typedef s16 FWORD;
// uint16 that describes a quantity in font design units.
typedef u16 UFWORD;

//16-bit signed fixed number with the low 14 bits of fraction (2.14).
typedef u16 F2DOT14; 

// Date and time represented in number of seconds since 12:00 midnight, January 1, 1904, UTC. The value is represented as a signed 64-bit integer.  
typedef s64 LONGDATETIME; 

typedef u8 Tag[4];

//  8-bit offset to a table, same as uint8, NULL offset = 0x00
typedef u8 Offset8;
//	Short offset to a table, same as uint16, NULL offset = 0x0000
typedef u16 Offset16;
//	24-bit offset to a table, same as uint24, NULL offset = 0x000000
typedef u8 Offset24[3];
//	Long offset to a table, same as uint32, NULL offset = 0x00000000
typedef u32 Offset32;

// The Version16Dot16 type is used for the version field of certain tables, and only for reasons of backward compatibility. 
// (In earlier versions, these fields were documented as using a Fixed value, but had minor version numbers that did not 
// follow the definition of the Fixed type.) Version16Dot16 is a packed value: the upper 16 bits comprise a major version 
// number, and the lower 16 bits, a minor version. Non-zero minor version numbers are represented using digits 0 to 9 in 
// the highest-order nibbles of the lower 16 bits. For example, the version field of 'maxp' table version 0.5 is 0x00005000, 
// and that of 'vhea' table version 1.1 is 0x00011000. This type is used only in the 'maxp', 'post' and 'vhea' tables, and will 
// not be used for any other tables in the future.

// Packed 32-bit value with major and minor version numbers.
struct Version16Dot16 {
    u16 major_version;
    u16 minor_version;
};


// Within this specification, many structures are defined in terms of the data types listed above. Structures are characterized as either records or tables. The distinction between records and tables is based on these general criteria:

//     Tables are referenced by offsets. If a table contains an offset to a sub-structure, the offset is normally from the start of that table.
//     Records occur sequentially within a parent structure, either within a sequence of table fields or within an array of records of a given type. If a record contains an offset to a sub-structure, that structure is logically a subtable of the record’s parent table and the offset is normally from the start of the parent table.


// Most tables have version numbers, and the version number for the entire font is contained in the table directory. Note that there are five different table version number types, each with its own numbering scheme.

    // A single uint16 field. This is used in a number of tables, usually with versions starting at zero (0).
    // Separate, uint16 major and minor version fields. This is used in a number of tables, usually with versions starting at 1.0.
    // A uint32 field with enumerated values.
    // A uint32 field with a numeric value. This is used only in the DSIG and 'meta' tables.
    // A Version16Dot16 field for major/minor version numbers. This is used only in the 'maxp', 'post' and 'vhea' tables.



char *file_to_str(const char *file_path);

f32 F2DOT14_to_f32(F2DOT14 a) {

    u32 sign = a >> 15;
    s8 unsigned_part = (a >> 14) & 0x1;
    s8 integer_part = -(s8)(sign << 1) + unsigned_part;

    u16 fractional_part = a & 0b0011111111111111;


    return (f32)integer_part + (f32)fractional_part / (f32)(1 << 14);
}
 

struct TableRecord {
    Tag tableTag; // Table identifier.
    u32 checksum; // Checksum for this table.
    Offset32 offset_from_start_of_file; // Offset from beginning of font file.
    u32 length; // Length of this table.
};


// big endian to little endian btl 
u16 btl_u16(u16 a) {

    u8 *a_ = (u8 *)&a;
    u8 a1 = *(a_ + 0); 
    u8 a0 = *(a_ + 1); 


    return (u16)((a1 << 8) | (a0 << 0));
}

u32 btl_u32(u32 a) {

    u8 *a_ = (u8 *)&a;
    u8 a3 = *(a_ + 0); 
    u8 a2 = *(a_ + 1); 
    u8 a1 = *(a_ + 2); 
    u8 a0 = *(a_ + 3); 

    u32 r =(u32) (
        (a3 << 24) | (a2 << 16) | (a1 << 8) | (a0 << 0)
    ); 
    return r; 
}

// table checksum function from microsoft
u32 CalcTableChecksum(u32 *Table, u32 Length) {
    u32 Sum = 0;
    u32 *EndPtr = Table + ((Length + 3) & ~3) / sizeof(u32);
    while (Table < EndPtr) {
        Sum += *Table++;
    }
    return Sum;
}

// All tables must begin on four-byte boundaries, and any remaining space between tables must be padded with zeros. 
// The length of each table should be recorded in the table record with the actual length of data, not the padded length.
int main(void) {

    printf("%f\n", F2DOT14_to_f32(0x7fff));
    printf("%f\n", F2DOT14_to_f32(0x7000));
    printf("%f\n", F2DOT14_to_f32(0x0001));
    printf("%f\n", F2DOT14_to_f32(0x0000));
    printf("%f\n", F2DOT14_to_f32(0xffff));
    printf("%f\n", F2DOT14_to_f32(0x8000));

    const char *path = "C:/windows/fonts/times.ttf";
    char *data = file_to_str(path);

    char *table_directory = data;

    u32 *sfntVersion = (u32 *)table_directory;
    if (btl_u32(*sfntVersion) == 0x00010000 || btl_u32(*sfntVersion) == 0x4F54544F) {
        u16 *numTables = (u16 *)(sfntVersion + 1);
        u16 *searchRange = numTables + 1;
        u16 *entrySelector = searchRange + 1;
        u16 *rangeShift = entrySelector + 1;


        TableRecord *tableRecords = (TableRecord *)(rangeShift + 1);
        u16 num_tables = btl_u16(*numTables);
        for (u64 i = 0; i < num_tables; ++i) {
            TableRecord *tr = tableRecords + i;
            printf("Table tag %c%c%c%c\n", tr->tableTag[0], tr->tableTag[1], tr->tableTag[2], tr->tableTag[3]); 
        }
    } else {
        LOG_ERROR("Invalid sfntVersion in font %s\n", path);
        return 1;
    }
    // TODO get information from required tables
// Tag 	Name
// 'cmap' 	Character to glyph mapping
// 'head' 	Font header
// 'hhea' 	Horizontal header
// 'hmtx' 	Horizontal metrics
// 'maxp' 	Maximum profile
// 'name' 	Naming table
// OS/2 	OS/2 and Windows specific metrics
// 'post' 	PostScript information
    return 0;   
}



#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


char *file_to_str(const char *file_path)
{
    char *str = nullptr;
    FILE *f = fopen(file_path, "rb");
    if (f == nullptr)
    {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return nullptr;
    }

    long file_size = -1;
    if (fseek(f, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "ERROR: failed to seek file %s\n", file_path);
        goto end_close;
    }
    file_size = ftell(f);
    if (file_size < 0)
    {
        goto end_close;
    }
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "ERROR: failed to seek file %s\n", file_path);
        goto end_close;
    }
    {
        u64 buf_size = (u64)file_size + 1; 
        str = (char *)malloc(buf_size);
    }
    if (fread(str, sizeof(*str), (u64)file_size, f) != (u64)file_size)
    {
        fprintf(stderr, "ERROR: failed to read data from file %s\n", file_path);
        str = nullptr;
    }

    end_close:
    if (fclose(f) == EOF)
    {
        fprintf(stderr, "ERROR: failed to close file %s\n", file_path);
    }
    return str;
}