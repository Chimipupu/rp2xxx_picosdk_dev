/**
 * @file ascii_code.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief ASCIIコード
 * @version 0.1
 * @date 2025-08-14
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#ifndef ASCII_CODE_H
#define ASCII_CODE_H

// ASCII (0x00-0x7F)
#define ASCII_NUL                 0x00  // Null
#define ASCII_SOH                 0x01  // Start of Heading
#define ASCII_STX                 0x02  // Start of Text
#define ASCII_ETX                 0x03  // End of Text
#define ASCII_EOT                 0x04  // End of Transmission
#define ASCII_ENQ                 0x05  // Enquiry
#define ASCII_ACK                 0x06  // Acknowledge
#define ASCII_BEL                 0x07  // Bell
#define ASCII_BS                  0x08  // Backspace
#define ASCII_TAB                 0x09  // Horizontal Tab
#define ASCII_LF                  0x0A  // Line Feed
#define ASCII_VT                  0x0B  // Vertical Tab
#define ASCII_FF                  0x0C  // Form Feed
#define ASCII_CR                  0x0D  // Carriage Return
#define ASCII_SO                  0x0E  // Shift Out
#define ASCII_SI                  0x0F  // Shift In
#define ASCII_DLE                 0x10  // Data Link Escape
#define ASCII_DC1                 0x11  // Device Control 1 (XON)
#define ASCII_DC2                 0x12  // Device Control 2
#define ASCII_DC3                 0x13  // Device Control 3 (XOFF)
#define ASCII_DC4                 0x14  // Device Control 4
#define ASCII_NAK                 0x15  // Negative Acknowledge
#define ASCII_SYN                 0x16  // Synchronous Idle
#define ASCII_ETB                 0x17  // End of Transmission Block
#define ASCII_CAN                 0x18  // Cancel
#define ASCII_EM                  0x19  // End of Medium
#define ASCII_SUB                 0x1A  // Substitute
#define ASCII_ESC                 0x1B  // Escape
#define ASCII_FS                  0x1C  // File Separator
#define ASCII_GS                  0x1D  // Group Separator
#define ASCII_RS                  0x1E  // Record Separator
#define ASCII_US                  0x1F  // Unit Separator

#define ASCII_SPACE               0x20  // Space
#define ASCII_EXCLAIM             0x21  // !
#define ASCII_DQUOTE              0x22  // "
#define ASCII_HASH                0x23  // #
#define ASCII_DOLLAR              0x24  // $
#define ASCII_PERCENT             0x25  // %
#define ASCII_AMPERSAND           0x26  // &
#define ASCII_SQUOTE              0x27  // '
#define ASCII_LPAREN              0x28  // (
#define ASCII_RPAREN              0x29  // )
#define ASCII_ASTERISK            0x2A  // *
#define ASCII_PLUS                0x2B  // +
#define ASCII_COMMA               0x2C  // ,
#define ASCII_MINUS               0x2D  // -
#define ASCII_PERIOD              0x2E  // .
#define ASCII_SLASH               0x2F  // /

#define ASCII_0                   0x30  // 0
#define ASCII_1                   0x31  // 1
#define ASCII_2                   0x32  // 2
#define ASCII_3                   0x33  // 3
#define ASCII_4                   0x34  // 4
#define ASCII_5                   0x35  // 5
#define ASCII_6                   0x36  // 6
#define ASCII_7                   0x37  // 7
#define ASCII_8                   0x38  // 8
#define ASCII_9                   0x39  // 9

#define ASCII_COLON               0x3A  // :
#define ASCII_SEMICOLON           0x3B  // ;
#define ASCII_LESS                0x3C  // <
#define ASCII_EQUAL               0x3D  // =
#define ASCII_GREATER             0x3E  // >
#define ASCII_QUESTION            0x3F  // ?
#define ASCII_AT                  0x40  // @

#define ASCII_A                   0x41  // A
#define ASCII_B                   0x42  // B
#define ASCII_C                   0x43  // C
#define ASCII_D                   0x44  // D
#define ASCII_E                   0x45  // E
#define ASCII_F                   0x46  // F
#define ASCII_G                   0x47  // G
#define ASCII_H                   0x48  // H
#define ASCII_I                   0x49  // I
#define ASCII_J                   0x4A  // J
#define ASCII_K                   0x4B  // K
#define ASCII_L                   0x4C  // L
#define ASCII_M                   0x4D  // M
#define ASCII_N                   0x4E  // N
#define ASCII_O                   0x4F  // O
#define ASCII_P                   0x50  // P
#define ASCII_Q                   0x51  // Q
#define ASCII_R                   0x52  // R
#define ASCII_S                   0x53  // S
#define ASCII_T                   0x54  // T
#define ASCII_U                   0x55  // U
#define ASCII_V                   0x56  // V
#define ASCII_W                   0x57  // W
#define ASCII_X                   0x58  // X
#define ASCII_Y                   0x59  // Y
#define ASCII_Z                   0x5A  // Z

#define ASCII_LBRACKET            0x5B  // [
#define ASCII_BACKSLASH           0x5C  // ＼
#define ASCII_RBRACKET            0x5D  // ]
#define ASCII_CARET               0x5E  // ^
#define ASCII_UNDERSCORE          0x5F  // _
#define ASCII_GRAVE               0x60  // `

#define ASCII_a                   0x61  // a
#define ASCII_b                   0x62  // b
#define ASCII_c                   0x63  // c
#define ASCII_d                   0x64  // d
#define ASCII_e                   0x65  // e
#define ASCII_f                   0x66  // f
#define ASCII_g                   0x67  // g
#define ASCII_h                   0x68  // h
#define ASCII_i                   0x69  // i
#define ASCII_j                   0x6A  // j
#define ASCII_k                   0x6B  // k
#define ASCII_l                   0x6C  // l
#define ASCII_m                   0x6D  // m
#define ASCII_n                   0x6E  // n
#define ASCII_o                   0x6F  // o
#define ASCII_p                   0x70  // p
#define ASCII_q                   0x71  // q
#define ASCII_r                   0x72  // r
#define ASCII_s                   0x73  // s
#define ASCII_t                   0x74  // t
#define ASCII_u                   0x75  // u
#define ASCII_v                   0x76  // v
#define ASCII_w                   0x77  // w
#define ASCII_x                   0x78  // x
#define ASCII_y                   0x79  // y
#define ASCII_z                   0x7A  // z

#define ASCII_LBRACE              0x7B  // {
#define ASCII_PIPE                0x7C  // |
#define ASCII_RBRACE              0x7D  // }
#define ASCII_TILDE               0x7E  // ~
#define ASCII_DELETE              0x7F  // Delete

#endif // ASCII_CODE_H