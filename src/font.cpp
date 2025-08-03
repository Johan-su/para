#include "common.h"
#include "string.h"
#include "arena.h"
#include "meta.h"
#include <stdlib.h>
#include <string.h>

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



String file_to_str(const char *file_path);

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

struct EncodingRecord {
    u16 platformID; // Platform ID
    u16 encodingId; // Platform-specific encoding ID
    Offset32 subtableOffset; // Byte offset from beginning of table to the subtable for this encoding
};


// big endian to little endian btl 
s16 btl_s16(s16 a) {

    u8 *a_ = (u8 *)&a;
    u8 a1 = *(a_ + 0); 
    u8 a0 = *(a_ + 1); 


    return (s16)((a1 << 8) | (a0 << 0));
}

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

bool is_tabletag(String stag, Tag table_tag) {
    if (stag.count != 4) return false;


    for (u64 i = 0; i < 4; ++i) {
        if (stag.dat[i] != table_tag[i]) return false;
    }
    return true;
}


#define FONT_PLATFORM_UNICODE 0 	// Unicode 	Various
#define FONT_PLATFORM_MACINTOSH 1 	// Macintosh 	Script manager code
#define FONT_PLATFORM_ISO 2 	    // ISO [deprecated] 	ISO encoding [deprecated]
#define FONT_PLATFORM_WINDOWS 3 	// Windows 	Windows encoding
#define FONT_PLATFORM_CUSTOM 4 	    // Custom 	Custom


#define FONT_UNICODE1_0 0 	// Unicode 1.0 semantics—deprecated
#define FONT_UNICODE1_1 1 	// Unicode 1.1 semantics—deprecated
#define FONT_UNICODE_ISO_IEC_10646 2 //ISO/IEC 10646 semantics—deprecated
#define FONT_UNICODE2_0_BMPONLY 3 //Unicode 2.0 and onwards semantics, Unicode BMP only
#define FONT_UNICODE2_0_FULL 4 //Unicode 2.0 and onwards semantics, Unicode full repertoire
#define FONT_UNICODE_VARIATIONSEQ_FOR_SUBTABLE_FORMAT14 5 //Unicode variation sequences—for use with subtable format 14
#define FONT_UNICODE_FULL_FOR_SUBTABLE_FORMAT13 6 // Unicode full repertoire—for use with subtable format 13


struct SequentialMapGroup {
    u32 startCharCode;
    u32 endCharCode;
    u32 startGlyphID;
};

struct Hmetrics {
    u16 advance_width; // in font design units.
    s16 left_side_bearing; // in font design units.
};

enum Contour_Flags: u8 {
    CONTOUR_FLAGS_ON_CURVE_POINT = 0x01, // Bit 0: If set, the point is on the curve; otherwise, it is off the curve.
    CONTOUR_FLAGS_X_SHORT_VECTOR = 0x02, // Bit 1: If set, the corresponding x-coordinate is 1 byte long, and the sign is determined by the X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR flag. If not set, its interpretation depends on the X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR flag: If that other flag is set, the x-coordinate is the same as the previous x-coordinate, and no element is added to the xCoordinates array. If both flags are not set, the corresponding element in the xCoordinates array is two bytes and interpreted as a signed integer. See the description of the X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR flag for additional information.
    CONTOUR_FLAGS_Y_SHORT_VECTOR = 0x04, // Bit 2: If set, the corresponding y-coordinate is 1 byte long, and the sign is determined by the Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR flag. If not set, its interpretation depends on the Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR flag: If that other flag is set, the y-coordinate is the same as the previous y-coordinate, and no element is added to the yCoordinates array. If both flags are not set, the corresponding element in the yCoordinates array is two bytes and interpreted as a signed integer. See the description of the Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR flag for additional information.
    CONTOUR_FLAGS_REPEAT_FLAG = 0x08, // Bit 3: If set, the next byte (read as unsigned) specifies the number of additional times this flag byte is to be repeated in the logical flags array — that is, the number of additional logical flag entries inserted after this entry. (In the expanded logical array, this bit is ignored.) In this way, the number of flags listed can be smaller than the number of points in the glyph description.
    CONTOUR_FLAGS_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR = 0x10, // Bit 4: This flag has two meanings, depending on how the X_SHORT_VECTOR flag is set. If X_SHORT_VECTOR is set, this bit describes the sign of the value, with 1 equaling positive and 0 negative. If X_SHORT_VECTOR is not set and this bit is set, then the current x-coordinate is the same as the previous x-coordinate. If X_SHORT_VECTOR is not set and this bit is also not set, the current x-coordinate is a signed 16-bit delta vector.
    CONTOUR_FLAGS_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR = 0x20, // Bit 5: This flag has two meanings, depending on how the Y_SHORT_VECTOR flag is set. If Y_SHORT_VECTOR is set, this bit describes the sign of the value, with 1 equaling positive and 0 negative. If Y_SHORT_VECTOR is not set and this bit is set, then the current y-coordinate is the same as the previous y-coordinate. If Y_SHORT_VECTOR is not set and this bit is also not set, the current y-coordinate is a signed 16-bit delta vector.
    CONTOUR_FLAGS_OVERLAP_SIMPLE = 0x40, // Bit 6: If set, contours in the glyph description could overlap. Use of this flag is not required — that is, contours may overlap without having this flag set. When used, it must be set on the first flag byte for the glyph. See additional details below.
    CONTOUR_FLAGS_Reserved = 0x80, // Bit 7 is reserved: set to zero.
};

struct ContourPoint {
    bool point_on_curve;
    s16 x_coord;
    s16 y_coord;
};

struct Glyph {
    s16 number_of_contours; // If the number of contours is greater than or equal to zero, this is a simple glyph. If negative, this is a composite glyph — the value -1 should be used for composite glyphs.
    s16 xmin; // Minimum x for coordinate data.
    s16 ymin; // Minimum y for coordinate data.
    s16 xmax; // Maximum x for coordinate data.
    s16 ymax; // Maximum y for coordinate data.


    DynArray<u16> end_pts_of_contour; // Array of point indices for the last point of each contour, in increasing numeric order.

    // u16 instruction_length; // Total number of bytes for instructions. If instructionLength is zero, no instructions are present for this glyph, and this field is followed directly by the flags field.
    // DynArray<u8> instructions; // Array of instruction byte code for the glyph.
    DynArray<ContourPoint> contour_points;
};


struct Font {

    DynArray<SequentialMapGroup> smgs;


// head
    bool baseline_at_y0;
    bool left_sidebearing_point_at_x0;
    bool instructions_may_depend_on_point_size;
    bool instructions_may_alter_advance_width;

    u16 unitsPerEm;
    s16 xMin;
    s16 yMin;
    s16 xMax;
    s16 yMax;

    // macStyle
    bool bold;
    bool italic;
    bool underline;
    bool outline;
    bool shadow;
    bool condensed;
    bool extended;

    s16 index_to_loc_format; // 0 for short offsets (Offset16), 1 for long (Offset32)

// maxp

    u16 num_glyphs;


// loca

    DynArray<Offset32> loca_offsets;
// glyf

    DynArray<Glyph> glyphs;
//

    u16 advance_width_max;
    s16 min_left_side_bearing;
    s16 min_right_side_bearing;
    s16 x_max_extent;

    s16 caret_slope_rise;
    s16 caret_slope_run;
    s16 caret_offset;

    DynArray<Hmetrics> glyph_hmetrics;
    DynArray<s16> glyph_left_side_bearings;


    s16 xAvgCharWidth; 
    u16 usWeightClass; 
    u16 usWidthClass; 
    u16 fsType; 
    s16 ySubscriptXSize; 
    s16 ySubscriptYSize; 
    s16 ySubscriptXOffset; 
    s16 ySubscriptYOffset; 
    s16 ySuperscriptXSize; 
    s16 ySuperscriptYSize; 
    s16 ySuperscriptXOffset; 
    s16 ySuperscriptYOffset; 
    s16 yStrikeoutSize; 
    s16 yStrikeoutPosition; 
    s16 sFamilyClass; 
    u8 panose[10]; 
    u32 ulUnicodeRange1; 
    u32 ulUnicodeRange2; 
    u32 ulUnicodeRange3; 
    u32 ulUnicodeRange4; 
    Tag achVendID; 
    u16 fsSelection; 
    u16 usFirstCharIndex; 
    u16 usLastCharIndex; 
    s16 sTypoAscender; 
    s16 sTypoDescender; 
    s16 sTypoLineGap; 
    u16 usWinAscent; 
    u16 usWinDescent; 
    u32 ulCodePageRange1; 
    u32 ulCodePageRange2; 
    s16 sxHeight; 
    s16 sCapHeight; 
    u16 usDefaultChar; 
    u16 usBreakChar; 
    u16 usMaxContext; 
    u16 usLowerOpticalPointSize; 
    u16 usUpperOpticalPointSize; 


};



void parse_cmap(Font *font, u8 *table_directory, Offset32 tr_offset_from_start_of_file) {
    u8 *cmap_table = table_directory + tr_offset_from_start_of_file;

    u16 *version = (u16 *)cmap_table;
    u16 *numsubTables = version + 1;
    EncodingRecord *bigendian_encodingRecords = (EncodingRecord *)(numsubTables + 1);

    if (btl_u16(*version) == 0x0000) {
        u16 num_sub_tables = btl_u16((*numsubTables));
        for (u64 j = 0; j < num_sub_tables; ++j) {
            u16 er_platformID = btl_u16(bigendian_encodingRecords[j].platformID);
            u16 er_encodingId = btl_u16(bigendian_encodingRecords[j].encodingId);
            Offset32 er_subtableOffset = btl_u32(bigendian_encodingRecords[j].subtableOffset);
            if (er_platformID == FONT_PLATFORM_UNICODE) {
                switch (er_encodingId) {
                    case FONT_UNICODE1_0: {
                    } break;
                    case FONT_UNICODE1_1: {
                    } break;
                    case FONT_UNICODE_ISO_IEC_10646: {
                    } break;
                    case FONT_UNICODE2_0_BMPONLY: {
                    } break;
                    case FONT_UNICODE2_0_FULL: {
                        u8 *sub_table = cmap_table + er_subtableOffset;

                        u16 *format = (u16 * )sub_table;
                        u16 btl_format = btl_u16(*format);

                        switch (btl_format) {
                            case 12: {
                                u16 *reserved = format + 1;
                                u32 *length = (u32 *)(reserved + 1);
                                u32 *language = length + 1;
                                u32 *numGroups = language + 1;
                                SequentialMapGroup *smgs = (SequentialMapGroup *)(numGroups + 1); 
                                u32 num_groups = btl_u32(*numGroups);
                                for (u64 k = 0; k < num_groups; ++k) {
                                    u32 smg_startCharCode = smgs[k].startCharCode;
                                    u32 smg_endCharCode = smgs[k].endCharCode;
                                    u32 smg_startGlyphID = smgs[k].startGlyphID;
                                    printf("start %X end %X start glyph %X\n", smg_startCharCode, smg_endCharCode, smg_startGlyphID);

                                    SequentialMapGroup smg = {};
                                    smg.startCharCode = smg_startCharCode;
                                    smg.endCharCode = smg_endCharCode;
                                    smg.startGlyphID = smg_startGlyphID;

                                    dynarray_append(&font->smgs, smg);
                                }

                            } break;

                            default: assert(false && "unsupported format");
                        } 

                    } break;
                    case FONT_UNICODE_VARIATIONSEQ_FOR_SUBTABLE_FORMAT14: {
                    } break;
                    case FONT_UNICODE_FULL_FOR_SUBTABLE_FORMAT13: {
                    } break;
                }
            }
        }

    }
}

// All tables must begin on four-byte boundaries, and any remaining space between tables must be padded with zeros. 
// The length of each table should be recorded in the table record with the actual length of data, not the padded length.

struct TagIndex {
    Tag table_tag;
    u64 index;
};

int main(void) {

    Font font = {};
    DynArray<TagIndex> tag_to_table = {};

    printf("%f\n", F2DOT14_to_f32(0x7fff));
    printf("%f\n", F2DOT14_to_f32(0x7000));
    printf("%f\n", F2DOT14_to_f32(0x0001));
    printf("%f\n", F2DOT14_to_f32(0x0000));
    printf("%f\n", F2DOT14_to_f32(0xffff));
    printf("%f\n", F2DOT14_to_f32(0x8000));

    // const char *path = "./NewCM08-Regular.otf";
    const char *path = "C:/windows/fonts/arial.ttf";
    String data = file_to_str(path);

    u8 *table_directory = data.dat;

    u32 *sfntVersion = (u32 *)table_directory;
    u16 *numTables = (u16 *)(sfntVersion + 1);
    u16 *searchRange = numTables + 1;
    u16 *entrySelector = searchRange + 1;
    u16 *rangeShift = entrySelector + 1;
    TableRecord *bigendian_tableRecords = (TableRecord *)(rangeShift + 1);

    if (btl_u32(*sfntVersion) == 0x00010000 || btl_u32(*sfntVersion) == 0x4F54544F) {
        u16 num_tables = btl_u16(*numTables);

        u16 btl_numberOfHMetrics = 0;

        for (u64 i = 0; i < num_tables; ++i) {
            TagIndex ti = {};
            memcpy(ti.table_tag, bigendian_tableRecords[i].tableTag, sizeof(ti.table_tag));
            ti.index = i;
            dynarray_append(&tag_to_table, ti);
        }
        for (u64 i = 0; i < tag_to_table.count; ++i) {
            TagIndex *ti = tag_to_table.dat + i; 
            if (is_tabletag(str_lit("hhea"), ti->table_tag)) {
                dynarray_swap(&tag_to_table, 0, i);
            } else if (is_tabletag(str_lit("head"), ti->table_tag)) {
                dynarray_swap(&tag_to_table, 1, i);
            } else if (is_tabletag(str_lit("maxp"), ti->table_tag)) {
                dynarray_swap(&tag_to_table, 2, i);
            } else if (is_tabletag(str_lit("hmtx"), ti->table_tag)) {
                dynarray_swap(&tag_to_table, 3, i);
            } else if (is_tabletag(str_lit("loca"), ti->table_tag)) {
                dynarray_swap(&tag_to_table, 4, i);
            } else if (is_tabletag(str_lit("glyf"), ti->table_tag)) {
                dynarray_swap(&tag_to_table, 5, i);
            }
        }

        for (u64 i = 0; i < tag_to_table.count; ++i) {
            TagIndex *ti = tag_to_table.dat + i; 
            u64 index = ti->index; 

            Tag tr_tableTag; memcpy(tr_tableTag, bigendian_tableRecords[index].tableTag, sizeof(tr_tableTag));
            u32 tr_checksum = btl_u32(bigendian_tableRecords[index].checksum);
            Offset32 tr_offset_from_start_of_file = btl_u32(bigendian_tableRecords[index].offset_from_start_of_file);
            u32 tr_length = btl_u32(bigendian_tableRecords[index].length);

            if (is_tabletag(str_lit("cmap"), tr_tableTag)) {
                parse_cmap(&font, table_directory, tr_offset_from_start_of_file);
            } else if (is_tabletag(str_lit("head"), tr_tableTag)) {
                // This table gives global information about the font. 
                // The bounding box values should be computed using only glyphs that have contours. 
                // Glyphs with no contours should be ignored for the purposes of these calculations.
                u8 *head_table = table_directory + tr_offset_from_start_of_file;

                u16 *majorVersion = (u16 *)head_table;
                u16 *minorVersion = majorVersion + 1;
                Fixed *fontRevision = (Fixed *)(minorVersion + 1);
                // To compute: set it to 0, sum the entire font as uint32, then store 0xB1B0AFBA - sum. If the font is used as a component in a font collection file, the value of this field will be invalidated by changes to the file structure and font table directory, and must be ignored.
                u32 *checksumAdjustment = (u32 *)(fontRevision + 1);
                u32 *magicNumber = (checksumAdjustment + 1);
                u16 *flags = (u16 *)(magicNumber + 1);
                u16 *unitsPerEm = flags + 1;
                LONGDATETIME *created = (LONGDATETIME *)(unitsPerEm + 1);
                LONGDATETIME *modified = created + 1;
                s16 *xMin = (s16 *)(modified + 1);
                s16 *yMin = xMin + 1;
                s16 *xMax = yMin + 1;
                s16 *yMax = xMax + 1;
                u16 *macStyle = (u16 *)(yMax + 1);
                u16 *lowestRecPPEM = macStyle + 1;
                s16 *fontDirectionHint  = (s16 *)(lowestRecPPEM + 1);
                s16 *indexToLocFormat  = fontDirectionHint + 1;
                s16 *glyphDataFormat  = indexToLocFormat + 1;


                if (btl_u32(*magicNumber) == 0x5F0F3CF5) {

                    u16 btl_flags = btl_u16(*flags);

                    if (btl_flags & (1 << 0)) font.baseline_at_y0 = true;
                    if (btl_flags & (1 << 1)) font.left_sidebearing_point_at_x0 = true;
                    if (btl_flags & (1 << 2)) font.instructions_may_depend_on_point_size = true;
                    // if (btl_flags & (1 << 3))
                    if (btl_flags & (1 << 4)) font.instructions_may_alter_advance_width = true;



                    font.unitsPerEm = btl_u16(*unitsPerEm);

                    font.xMin = btl_s16(*xMin);
                    font.yMin = btl_s16(*yMin);
                    font.xMax = btl_s16(*xMax);
                    font.yMax = btl_s16(*yMax);

                    u16 btl_macStyle = btl_u16(*macStyle);

                    if (btl_macStyle & (1 << 0)) font.bold = true;
                    if (btl_macStyle & (1 << 1)) font.italic = true;
                    if (btl_macStyle & (1 << 2)) font.underline = true;
                    if (btl_macStyle & (1 << 3)) font.outline = true;
                    if (btl_macStyle & (1 << 4)) font.shadow = true;
                    if (btl_macStyle & (1 << 5)) font.condensed = true;
                    if (btl_macStyle & (1 << 6)) font.extended = true;

                    font.index_to_loc_format = btl_s16(*indexToLocFormat);

                } else {
                    LOG_ERROR("Invalid magic in `head` table in font %s\n", path);
                    return 1;
                }



            } else if (is_tabletag(str_lit("hhea"), tr_tableTag)) {
                // This table contains information for horizontal layout. 
                // The values in the minRightSidebearing, minLeftSideBearing and xMaxExtent should be computed using only glyphs that have contours. 
                // Glyphs with no contours should be ignored for the purposes of these calculations. All reserved areas must be set to 0.

                u8 *hhea_table = table_directory + tr_offset_from_start_of_file;
                

                u16 *majorVersion = (u16 *)hhea_table;
                u16 *minorVersion = (u16 *)(majorVersion + 1);

                FWORD *ascender = (FWORD *)(minorVersion + 1);
                FWORD *descender = (FWORD *)(ascender + 1);
                FWORD *lineGap = (FWORD *)(descender + 1);
                UFWORD *advanceWidthMax = (UFWORD *)(lineGap + 1);

                FWORD *minLeftSideBearing = (FWORD *)(advanceWidthMax + 1);
                FWORD *minRightSideBearing = (FWORD *)(minLeftSideBearing + 1);
                FWORD *xMaxExtent = (FWORD *)(minRightSideBearing + 1);

                s16 *caretSlopeRise = (s16 *)(xMaxExtent + 1);
                s16 *caretSlopeRun = (s16 *)(caretSlopeRise + 1);
                s16 *caretOffset = (s16 *)(caretSlopeRun + 1);

                s16 *reserved0 = (s16 *)(caretOffset + 1);
                s16 *reserved1 = (s16 *)(reserved0 + 1);
                s16 *reserved2 = (s16 *)(reserved1 + 1);
                s16 *reserved3 = (s16 *)(reserved2 + 1);

                s16 *metricDataFormat = (s16 *)(reserved3 + 1);
                u16 *numberOfHMetrics = (u16 *)(metricDataFormat + 1);

                
                font.advance_width_max = btl_u16(*advanceWidthMax);
                font.min_left_side_bearing = btl_s16(*minLeftSideBearing);
                font.min_right_side_bearing = btl_s16(*minRightSideBearing);
                font.x_max_extent = btl_s16(*xMaxExtent);
                font.caret_slope_rise = btl_s16(*caretSlopeRise);
                font.caret_slope_run = btl_s16(*caretSlopeRun);
                font.caret_offset = btl_s16(*caretOffset);


                btl_numberOfHMetrics = btl_u16(*numberOfHMetrics);
            } else if (is_tabletag(str_lit("hmtx"), tr_tableTag)) {
                u8 *hmtx_table = table_directory + tr_offset_from_start_of_file;

                Hmetrics *metrics_table = (Hmetrics *)hmtx_table;
                for (u64 j = 0; j < btl_numberOfHMetrics; ++j) {
                    Hmetrics *bigendian_metrics = metrics_table + j;


                    Hmetrics h_metrics = {};
                    h_metrics.advance_width = btl_u16(bigendian_metrics->advance_width);
                    h_metrics.left_side_bearing = btl_s16(bigendian_metrics->left_side_bearing);

                    dynarray_append(&font.glyph_hmetrics, h_metrics);
                }
                s16 *left_side_bearings = (s16 *)(hmtx_table + btl_numberOfHMetrics);

                u64 left_side_bearings_count = font.num_glyphs - btl_numberOfHMetrics;
                for (u64 j = 0; j < left_side_bearings_count; ++j) {
                    s16 *bigendian_lsb = left_side_bearings + j;
                    dynarray_append(&font.glyph_left_side_bearings, btl_s16(*bigendian_lsb));
                }
            } else if (is_tabletag(str_lit("maxp"), tr_tableTag)) {
                u8 *maxp_table = table_directory + tr_offset_from_start_of_file;
                
                Version16Dot16 *version = (Version16Dot16 *)maxp_table;
                u16 *numGlyphs = (u16 *)(version + 1);
                u16 *maxPoints = numGlyphs + 1;
                u16 *maxContours = maxPoints + 1;
                u16 *maxCompositePoints = maxContours + 1;
                u16 *maxCompositeContours = maxCompositePoints + 1;
                u16 *maxZones = maxCompositeContours + 1;
                u16 *maxTwilightPoints = maxZones + 1;
                u16 *maxStorage = maxTwilightPoints + 1;
                u16 *maxFunctionDefs = maxStorage + 1;
                u16 *maxInstructionDefs = maxFunctionDefs + 1;
                u16 *maxStackElements = maxInstructionDefs + 1;
                u16 *maxSizeOfInstructions = maxStackElements + 1;
                u16 *maxComponentElements = maxSizeOfInstructions + 1;
                u16 *maxComponentDepth = maxComponentElements + 1;

                font.num_glyphs = btl_u16(*numGlyphs);

            } else if (is_tabletag(str_lit("name"), tr_tableTag)) {
                // might be used later
            } else if (is_tabletag(str_lit("OS/2"), tr_tableTag)) {
                u8 *OS_slash2_table = table_directory + tr_offset_from_start_of_file;

                u16 *version = (u16 *)OS_slash2_table;
                FWORD *xAvgCharWidth = (FWORD *)(version + 1); 	
                u16 *usWeightClass = (u16 *)(xAvgCharWidth + 1); 	
                u16 *usWidthClass = (u16 *)(usWeightClass + 1); 	
                u16 *fsType = (u16 *)(usWidthClass + 1); 	
                FWORD *ySubscriptXSize = (FWORD *)(fsType + 1); 	
                FWORD *ySubscriptYSize = (FWORD *)(ySubscriptXSize + 1); 	
                FWORD *ySubscriptXOffset = (FWORD *)(ySubscriptYSize + 1); 	
                FWORD *ySubscriptYOffset = (FWORD *)(ySubscriptXOffset + 1); 	
                FWORD *ySuperscriptXSize = (FWORD *)(ySubscriptYOffset + 1); 	
                FWORD *ySuperscriptYSize = (FWORD *)(ySuperscriptXSize + 1); 	
                FWORD *ySuperscriptXOffset = (FWORD *)(ySuperscriptYSize + 1); 	
                FWORD *ySuperscriptYOffset = (FWORD *)(ySuperscriptXOffset + 1); 	
                FWORD *yStrikeoutSize = (FWORD *)(ySuperscriptYOffset + 1); 	
                FWORD *yStrikeoutPosition = (FWORD *)(yStrikeoutSize + 1); 	
                s16 *sFamilyClass = (s16 *)(yStrikeoutPosition + 1); 	
                u8 (*panose)[10] = (u8 (*)[10])(sFamilyClass + 1);
                u32 *ulUnicodeRange1 = (u32 *)(panose + 1);
                u32 *ulUnicodeRange2 = (u32 *)(ulUnicodeRange1 + 1);
                u32 *ulUnicodeRange3 = (u32 *)(ulUnicodeRange2 + 1);
                u32 *ulUnicodeRange4 = (u32 *)(ulUnicodeRange3 + 1);
                Tag *achVendID = (Tag *)(ulUnicodeRange4 + 1); 	
                u16 *fsSelection = (u16 *)(achVendID + 1); 	
                u16 *usFirstCharIndex = (u16 *)(fsSelection + 1); 	
                u16 *usLastCharIndex = (u16 *)(usFirstCharIndex + 1); 	
                FWORD *sTypoAscender = (FWORD *)(usLastCharIndex + 1); 	
                FWORD *sTypoDescender = (FWORD *)(sTypoAscender + 1); 	
                FWORD *sTypoLineGap = (FWORD *)(sTypoDescender + 1); 	
                UFWORD *usWinAscent = (UFWORD *)(sTypoLineGap + 1); 	
                UFWORD *usWinDescent = (UFWORD *)(usWinAscent + 1); 	
                u32 *ulCodePageRange1 = (u32 *)(usWinDescent + 1);
                u32 *ulCodePageRange2 = (u32 *)(ulCodePageRange1 + 1);
                FWORD *sxHeight = (FWORD *)(ulCodePageRange2 + 1); 	
                FWORD *sCapHeight = (FWORD *)(sxHeight + 1); 	
                u16 *usDefaultChar = (u16 *)(sCapHeight + 1); 	
                u16 *usBreakChar = (u16 *)(usDefaultChar + 1); 	
                u16 *usMaxContext = (u16 *)(usBreakChar + 1);
                u16 *usLowerOpticalPointSize = (u16 *)(usMaxContext + 1); 	
                u16 *usUpperOpticalPointSize = (u16 *)(usLowerOpticalPointSize + 1);






                font.xAvgCharWidth = btl_s16(*xAvgCharWidth); 
                font.usWeightClass = btl_u16(*usWeightClass); 
                font.usWidthClass = btl_u16(*usWidthClass); 
                font.fsType = btl_u16(*fsType); 
                font.ySubscriptXSize = btl_s16(*ySubscriptXSize); 
                font.ySubscriptYSize = btl_s16(*ySubscriptYSize); 
                font.ySubscriptXOffset = btl_s16(*ySubscriptXOffset); 
                font.ySubscriptYOffset = btl_s16(*ySubscriptYOffset); 
                font.ySuperscriptXSize = btl_s16(*ySuperscriptXSize); 
                font.ySuperscriptYSize = btl_s16(*ySuperscriptYSize); 
                font.ySuperscriptXOffset = btl_s16(*ySuperscriptXOffset); 
                font.ySuperscriptYOffset = btl_s16(*ySuperscriptYOffset); 
                font.yStrikeoutSize = btl_s16(*yStrikeoutSize); 
                font.yStrikeoutPosition = btl_s16(*yStrikeoutPosition); 
                font.sFamilyClass = btl_s16(*sFamilyClass); 
                memcpy(font.panose, panose, sizeof(*panose));
                font.ulUnicodeRange1 = btl_u32(*ulUnicodeRange1); 
                font.ulUnicodeRange2 = btl_u32(*ulUnicodeRange2); 
                font.ulUnicodeRange3 = btl_u32(*ulUnicodeRange3); 
                font.ulUnicodeRange4 = btl_u32(*ulUnicodeRange4); 
                memcpy(font.achVendID, achVendID, sizeof(*achVendID));
                font.fsSelection = btl_u16(*fsSelection); 
                font.usFirstCharIndex = btl_u16(*usFirstCharIndex); 
                font.usLastCharIndex = btl_u16(*usLastCharIndex); 
                font.sTypoAscender = btl_s16(*sTypoAscender); 
                font.sTypoDescender = btl_s16(*sTypoDescender); 
                font.sTypoLineGap = btl_s16(*sTypoLineGap); 
                font.usWinAscent = btl_u16(*usWinAscent); 
                font.usWinDescent = btl_u16(*usWinDescent); 
                font.ulCodePageRange1 = btl_u32(*ulCodePageRange1); 
                font.ulCodePageRange2 = btl_u32(*ulCodePageRange2); 
                font.sxHeight = btl_s16(*sxHeight); 
                font.sCapHeight = btl_s16(*sCapHeight); 
                font.usDefaultChar = btl_u16(*usDefaultChar); 
                font.usBreakChar = btl_u16(*usBreakChar); 
                font.usMaxContext = btl_u16(*usMaxContext); 
                if (btl_u16(*version) == 5) {
                    font.usLowerOpticalPointSize = btl_u16(*usLowerOpticalPointSize); 
                    font.usUpperOpticalPointSize = btl_u16(*usUpperOpticalPointSize);
                }
                	


            } else if (is_tabletag(str_lit("post"), tr_tableTag)) {

                // u8 *post_table = table_directory + tr_offset_from_start_of_file;

                // Version16Dot16 *version = (Version16Dot16 *)post_table;
                // Fixed *italicAngle = (Fixed *)(version + 1);
                // FWORD *underlinePosition = (FWORD *)(italicAngle + 1);
                // FWORD *underlineThickness = (FWORD *)(underlinePosition + 1);
                // u32 *isFixedPitch = (u32 *)(underlineThickness + 1);
                // u32 *minMemType42 = (u32 *)(isFixedPitch + 1);
                // u32 *maxMemType42 = (u32 *)(minMemType42 + 1);
                // u32 *minMemType1 = (u32 *)(maxMemType42 + 1);
                // u32 *maxMemType1 = (u32 *)(minMemType1 + 1);


            } else if (is_tabletag(str_lit("loca"), tr_tableTag)) {
                u8 *loca_table = table_directory + tr_offset_from_start_of_file;


                switch (font.index_to_loc_format) {
                    case 0: {
                        u64 offset_count = font.num_glyphs + 1;
                        Offset16 *offset_start = (Offset16 *)loca_table;
                        for (u64 j = 0; j < offset_count; ++j) {
                            Offset32 offset = 2 * (u32)btl_u16(offset_start[j]);
                            dynarray_append(&font.loca_offsets, offset);
                        }
                    } break;
                    case 1: {
                        u64 offset_count = font.num_glyphs + 1;
                        Offset32 *offset_start = (Offset32 *)loca_table;
                        for (u64 j = 0; j < offset_count; ++j) {
                            Offset32 *offsets = offset_start + j;
                            dynarray_append(&font.loca_offsets, btl_u32(*offsets));
                        }

                    } break;
                    default: assert(false && "unreachable");
                }


            } else if (is_tabletag(str_lit("glyf"), tr_tableTag)) {
                u8 *glyf_table = table_directory + tr_offset_from_start_of_file;
                assert((u64)glyf_table % 2 == 0);

                for (u64 glyph_id = 0; glyph_id < font.num_glyphs; ++glyph_id) {

                    u8 *glyph_start = glyf_table + font.loca_offsets.dat[glyph_id];
                    assert((u64)glyph_start % 2 == 0);

                    s16 *numberOfContours = (s16 *)glyph_start;
                    s16 *xMin = numberOfContours + 1;
                    s16 *yMin = xMin + 1;
                    s16 *xMax = yMin + 1;
                    s16 *yMax = xMax + 1;

                    Glyph glyph = {};

                    glyph.number_of_contours = btl_s16(*numberOfContours);
                    glyph.xmin = btl_s16(*xMin);
                    glyph.ymin = btl_s16(*yMin);
                    glyph.xmax = btl_s16(*xMax);
                    glyph.ymax = btl_s16(*yMax);


                    if (glyph.number_of_contours >= 0) {
                        u16 *endPtsOfContours = (u16 *)(yMax + 1);
                        u16 *instruction_length = endPtsOfContours + glyph.number_of_contours;
                        u16 btl_instruction_length = btl_u16(*instruction_length);
                        // assert(btl_instruction_length == 0);

                        u8 *instructions = (u8 *)(instruction_length + 1);
                        u8 *flags = instructions + btl_instruction_length;

                        
                        u16 number_of_contours = (u16)glyph.number_of_contours; 
                        for (u64 i3 = 0; i3 < number_of_contours; ++i3) {
                            dynarray_append(&glyph.end_pts_of_contour, btl_u16(endPtsOfContours[i3]));
                        }
                        u64 number_of_points = glyph.end_pts_of_contour.dat[glyph.end_pts_of_contour.count - 1] + 1;
                        DynArray<u8> flag_arr = {};
                        u64 flag_iter = 0;
                        for (u64 i3 = 0; i3 < number_of_points; ++i3) {
                            u8 f = flags[flag_iter];
                            dynarray_append(&flag_arr, f);
                            
                            if (f & CONTOUR_FLAGS_REPEAT_FLAG) {
                                u8 repeat = flags[flag_iter + 1];
                                for (u64 i4 = 0; i4 < repeat; ++i4) {
                                    dynarray_append(&flag_arr, f);
                                }
                                i3 += repeat;
                                flag_iter += 2;
                            } else {
                                flag_iter += 1;
                            }
                        }
                        u64 real_flag_len = flag_iter;
                        u8 *xCoordinates = flags + real_flag_len;

                        DynArray<s16> x_arr = {};
                        DynArray<s16> y_arr = {};

                        u64 x_index = 0;
                        s16 x = 0;
                        for (u64 i3 = 0; i3 < flag_arr.count; ++i3) {
                            u8 f = flag_arr.dat[i3];
                            
                            if (f & CONTOUR_FLAGS_X_SHORT_VECTOR) {

                                s16 dx = (f & CONTOUR_FLAGS_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) ? xCoordinates[x_index] : -xCoordinates[x_index];
                                x += dx;
                                dynarray_append(&x_arr, x);
                                x_index += 1;
                            } else {
                                if (f & CONTOUR_FLAGS_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                                    dynarray_append(&x_arr, x); 
                                } else {
                                    u8 *dx_coord = xCoordinates + x_index;
                                    
                                    s16 s_dx_coord;
                                    memcpy(&s_dx_coord, dx_coord, sizeof(s_dx_coord));

                                    s16 btl_dx_coord = btl_s16(s_dx_coord);
                                    x += btl_dx_coord;
                                    assert(x >= glyph.xmin);
                                    assert(x <= glyph.xmax);
                                    dynarray_append(&x_arr, x);
                                    x_index += 2;
                                }
                            }
                        }

                        u8 *yCoordinates = xCoordinates + x_index;

                        u64 y_index = 0;
                        s16 y = 0;
                        for (u64 i3 = 0; y_arr.count < flag_arr.count; ++i3) {
                            u8 f = flag_arr.dat[i3];
                            
                            if (f & CONTOUR_FLAGS_Y_SHORT_VECTOR) {

                                s16 dy = (f & CONTOUR_FLAGS_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) ? yCoordinates[y_index] : -yCoordinates[y_index];
                                y += dy;

                                dynarray_append(&y_arr, y);
                                y_index += 1;
                            } else {
                                if (f & CONTOUR_FLAGS_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                                    dynarray_append(&y_arr, y); 
                                } else {
                                    u8 *dy_coord = yCoordinates + y_index;
                                    
                                    s16 s_dy_coord;
                                    memcpy(&s_dy_coord, dy_coord, sizeof(s_dy_coord));

                                    s16 btl_dy_coord = btl_s16(s_dy_coord);
                                    y += btl_dy_coord;
                                    assert(y >= glyph.ymin);
                                    assert(y <= glyph.ymax);
                                    dynarray_append(&y_arr, y);
                                    y_index += 2;
                                }
                            }
                        }


                    } else {
                        todo();
                    }
                }

            }


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


String file_to_str(const char *file_path)
{
    String s = {};

    FILE *f = fopen(file_path, "rb");
    if (f == nullptr)
    {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return s;
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
        s.count = (u64)file_size;
        s.dat = (u8 *)malloc(s.count + 1);
    }
    if (fread(s.dat, sizeof(*s.dat), s.count, f) != s.count)
    {
        fprintf(stderr, "ERROR: failed to read data from file %s\n", file_path);
        s = {};
    }

    end_close:
    if (fclose(f) == EOF)
    {
        fprintf(stderr, "ERROR: failed to close file %s\n", file_path);
    }
    return s;
}