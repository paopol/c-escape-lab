#ifndef ESCAPE_H
#define ESCAPE_H

#include <stdio.h>
#include <stdlib.h>


#define ESCAPE_ESCAPE

#ifndef ESCAPE_DEBUG
#define ESCAPE_DEBUG 1
#endif


/* TODO LIST:
 *
 * ==================================================================
 * Ready to implement (macro-only, following existing patterns)
 * ==================================================================
 *
 * [CSI: _SSTR variants]
 * - EST_CSI_SM_ANSI_STANDARD_MODE_SSTR(Ps)
 * - EST_CSI_SM_DEC_PRIVATE_MODE_SSTR(Ps)
 * - EST_CSI_RM_ANSI_STANDARD_MODE_SSTR(Ps)
 * - EST_CSI_RM_DEC_PRIVATE_MODE_SSTR(Ps)
 * - EST_CSI_DECRQM_SSTR(Ps)
 * - EST_CSI_DECRQM_ANSI_STANDARD_MODE_SSTR(Ps)
 * - EST_CSI_DECRQM_DEC_PRIVATE_MODE_SSTR(Ps)
 *
 * [OSC: semantic wrappers]
 * Format: OSC Ps ; Pt ST
 * - EST_OSC_SET_TITLE_STR(title)            Ps=0
 * - EST_OSC_SET_ICON_STR(name)              Ps=1
 * - EST_OSC_SET_WINDOW_TITLE_STR(title)     Ps=2
 * - EST_OSC_SET_PALETTE_STR(idx, rgb)       Ps=4
 * - EST_OSC_RESET_PALETTE_STR(idx)          Ps=104
 * - EST_OSC_SET_CWD_STR(url)                Ps=7
 * - EST_OSC_HYPERLINK_OPEN_STR(url)         Ps=8
 * - EST_OSC_HYPERLINK_CLOSE_STR()           Ps=8
 * - EST_OSC_SET_FG_COLOR_STR(rgb)           Ps=10
 * - EST_OSC_SET_BG_COLOR_STR(rgb)           Ps=11
 * - EST_OSC_SET_CURSOR_COLOR_STR(rgb)       Ps=12
 * - EST_OSC_RESET_FG_COLOR_STR()            Ps=110
 * - EST_OSC_RESET_BG_COLOR_STR()            Ps=111
 * - EST_OSC_RESET_CURSOR_COLOR_STR()        Ps=112
 *
 * [ESC: single-character sequences]
 * - EST_ESC_RIS_STR()             ESC c   (full reset)
 * - EST_ESC_DEC_GRAPHICS_STR()    ESC ( 0
 * - EST_ESC_ASCII_STR()           ESC ( B
 * - EST_ESC_APP_KEYPAD_STR()      ESC =
 * - EST_ESC_NUM_KEYPAD_STR()      ESC >
 * - EST_ESC_IND_STR()             ESC D
 * - EST_ESC_NEL_STR()             ESC E
 * - EST_ESC_RI_STR()              ESC M
 * - EST_ESC_HTS_STR()             ESC H
 *
 * [DCS: semantic wrappers]
 * Format: DCS ... ST
 * - EST_DCS_DECRQSS_STR(setting)
 *
 * ==================================================================
 * Requires design (do not implement yet)
 * ==================================================================
 *
 * [Runtime I/O layer]
 * - Query APIs for DSR / CPR / DA1 / DA2 / XTVERSION / DECRQM:
 *     send query, read response, parse result.
 *   Open: blocking / non-blocking / timeout?
 *         read from stdin or /dev/tty or a user fd?
 *         portability (termios / Windows console)?
 *
 * - Output helper: ESCAPE_WRITE / ESCAPE_FLUSH
 *   Open: target stdout / stderr / arbitrary FILE *?
 *         flush policy?
 *
 * [Terminal capability detection]
 * - ESCAPE_SUPPORTS_256 / ESCAPE_SUPPORTS_TRUECOLOR
 *   Open: env vars (COLORTERM / TERM) or runtime query?
 *         cache or probe each time?
 *
 * [Other]
 * - ...
 */

#ifdef __cplusplus
extern "C" {
#endif



#pragma region "ESCAPE UTIL"

/* ABOUT `...` */

#ifndef ESCAPE_COMPAT_HAS_VA_OPT
    #if defined(__cplusplus)
        #if defined(__cpp_va_opt) && __cpp_va_opt >= 201907L
            #define ESCAPE_COMPAT_HAS_VA_OPT 1
        #elif defined(_MSC_VER) && _MSC_VER >= 1925 \
              && defined(_MSVC_TRADITIONAL) && !_MSVC_TRADITIONAL
            #define ESCAPE_COMPAT_HAS_VA_OPT 1
        #else
            #define ESCAPE_COMPAT_HAS_VA_OPT 0
        #endif
    #elif defined(__STDC_VERSION__)
        #if __STDC_VERSION__ >= 202311L
            #define ESCAPE_COMPAT_HAS_VA_OPT 1
        #elif defined(_MSC_VER) && _MSC_VER >= 1925 \
              && defined(_MSVC_TRADITIONAL) && !_MSVC_TRADITIONAL
            #define ESCAPE_COMPAT_HAS_VA_OPT 1
        #else
            #define ESCAPE_COMPAT_HAS_VA_OPT 0
        #endif
    #else
        #define ESCAPE_COMPAT_HAS_VA_OPT 0
    #endif
#endif

#ifndef ESCAPE_COMPAT_COMMA_VA_ARGS
    #if ESCAPE_COMPAT_HAS_VA_OPT
        /* Standard: expands to ", __VA_ARGS__" if non-empty, otherwise nothing. */
        #define ESCAPE_COMPAT_COMMA_VA_ARGS(...) __VA_OPT__(,) __VA_ARGS__
    #elif defined(__GNUC__) || defined(__clang__)
        /* GNU extension: ## removes the preceding comma when empty. */
        #define ESCAPE_COMPAT_COMMA_VA_ARGS(...) , ##__VA_ARGS__
    #else
        #error "Neither __VA_OPT__ nor ##__VA_ARGS__ is available. Define ESCAPE_COMPAT_COMMA_VA_ARGS manually."
    #endif
#endif

#ifndef ESCAPE_COMMA_VA_ARGS
#define ESCAPE_COMMA_VA_ARGS ESCAPE_COMPAT_COMMA_VA_ARGS
#endif

/* ABOUT `typeof` */

#ifndef ESCAPE_COMPAT_TYPEOF
    #if defined(__cplusplus)
        #if defined(_MSC_VER) && _MSC_VER >= 1600
            #define ESCAPE_COMPAT_TYPEOF(...) decltype(__VA_ARGS__)
        #elif __cplusplus >= 201103L
            #define ESCAPE_COMPAT_TYPEOF(...) decltype(__VA_ARGS__)
        #elif defined(__GNUC__) || defined(__clang__)
            #define ESCAPE_COMPAT_TYPEOF(...) __typeof__(__VA_ARGS__)
        #else
            #error "typeof is unavailable in this C++ compiler. Define ESCAPE_COMPAT_TYPEOF manually or use an explicit type."
        #endif
    #else
        #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
            #define ESCAPE_COMPAT_TYPEOF(...) typeof(__VA_ARGS__)
        #elif defined(__GNUC__) || defined(__clang__)
            #define ESCAPE_COMPAT_TYPEOF(...) __typeof__(__VA_ARGS__)
        #else
            #error "typeof is unavailable in this C compiler. Define ESCAPE_COMPAT_TYPEOF manually or use an explicit type."
        #endif
    #endif
#endif

#ifndef ESCAPE_TYPEOF
#define ESCAPE_TYPEOF ESCAPE_COMPAT_TYPEOF
#endif

/* ABOUT `#` and `##` */

#define ESCAPE__S(_)  #_
#define ESCAPE_S(_)   ESCAPE__S(_)

#define ESCAPE__C(_x, _y) _x##_y
#define ESCAPE_C(_x, _y)  ESCAPE__C(_x, _y)

#define ESCAPE_EMPTY ((void)0)

#define ESCAPE_APPLY(_f, ...) _f(__VA_ARGS__)

/* ABOUT `X` */

#define ESCAPE_X_DISPATCH_I(_1, _2, _3, _4, _5, _6, _7, _8, _x, ...) _x
#define ESCAPE_X_DISPATCH(...) ESCAPE_X_DISPATCH_I(__VA_ARGS__)
#define ESCAPE_X_ARG_CNT(...) \
    ESCAPE_X_DISPATCH(_ ESCAPE_COMMA_VA_ARGS(__VA_ARGS__), 7, 6, 5, 4, 3, 2, 1, 0)
#define ESCAPE_X_NAME(_prefix, ...) \
    ESCAPE_C(_prefix, ESCAPE_X_ARG_CNT(__VA_ARGS__))
#define ESCAPE_X_I(...) \
    ESCAPE_X_NAME(__VA_ARGS__)
#define ESCAPE_X(_prefix, ...) \
    ESCAPE_X_I(_prefix ESCAPE_COMMA_VA_ARGS(__VA_ARGS__))(__VA_ARGS__)

/* ABOUT `JOIN` */

#define ESCAPE_JOIN_I(_join_x, ...) _join_x(__VA_ARGS__)
#define ESCAPE_JOIN(_separator, ...) ESCAPE_JOIN_I(ESCAPE_C(ESCAPE_JOIN_, ESCAPE_X_ARG_CNT(__VA_ARGS__)), _separator ESCAPE_COMMA_VA_ARGS(__VA_ARGS__))
#define ESCAPE_JOIN_0(_separator)                               ""
#define ESCAPE_JOIN_1(_separator, _1)                           _1
#define ESCAPE_JOIN_2(_separator, _1, _2)                       _1 _separator _2
#define ESCAPE_JOIN_3(_separator, _1, _2, _3)                   _1 _separator _2 _separator _3
#define ESCAPE_JOIN_4(_separator, _1, _2, _3, _4)               _1 _separator _2 _separator _3 _separator _4
#define ESCAPE_JOIN_5(_separator, _1, _2, _3, _4, _5)           _1 _separator _2 _separator _3 _separator _4 _separator _5
#define ESCAPE_JOIN_6(_separator, _1, _2, _3, _4, _5, _6)       _1 _separator _2 _separator _3 _separator _4 _separator _5 _separator _6
#define ESCAPE_JOIN_7(_separator, _1, _2, _3, _4, _5, _6, _7)   _1 _separator _2 _separator _3 _separator _4 _separator _5 _separator _6 _separator _7

#define ESCAPE_JOINS(_separator, ...) ESCAPE_JOIN_I(ESCAPE_C(ESCAPE_JOINS_, ESCAPE_X_ARG_CNT(__VA_ARGS__)), _separator ESCAPE_COMMA_VA_ARGS(__VA_ARGS__))
#define ESCAPE_JOINS_0(_separator)                               ""
#define ESCAPE_JOINS_1(_separator, _1)                           ESCAPE_S(_1)
#define ESCAPE_JOINS_2(_separator, _1, _2)                       ESCAPE_S(_1) _separator ESCAPE_S(_2)
#define ESCAPE_JOINS_3(_separator, _1, _2, _3)                   ESCAPE_S(_1) _separator ESCAPE_S(_2) _separator ESCAPE_S(_3)
#define ESCAPE_JOINS_4(_separator, _1, _2, _3, _4)               ESCAPE_S(_1) _separator ESCAPE_S(_2) _separator ESCAPE_S(_3) _separator ESCAPE_S(_4)
#define ESCAPE_JOINS_5(_separator, _1, _2, _3, _4, _5)           ESCAPE_S(_1) _separator ESCAPE_S(_2) _separator ESCAPE_S(_3) _separator ESCAPE_S(_4) _separator ESCAPE_S(_5)
#define ESCAPE_JOINS_6(_separator, _1, _2, _3, _4, _5, _6)       ESCAPE_S(_1) _separator ESCAPE_S(_2) _separator ESCAPE_S(_3) _separator ESCAPE_S(_4) _separator ESCAPE_S(_5) _separator ESCAPE_S(_6)
#define ESCAPE_JOINS_7(_separator, _1, _2, _3, _4, _5, _6, _7)   ESCAPE_S(_1) _separator ESCAPE_S(_2) _separator ESCAPE_S(_3) _separator ESCAPE_S(_4) _separator ESCAPE_S(_5) _separator ESCAPE_S(_6) _separator ESCAPE_S(_7)

/* ABOUT `ASSERT` */

#define ESCAPE_ASSERT(_expression) \
    do { \
        if (!(_expression)) { \
            fprintf(stderr, "%s:%d: [ASSERT]: %s", __FILE__, __LINE__, ESCAPE_S(_expression)); \
            abort(); \
        } \
    } while (0)

/* ABOUT `TODO` */

#define ESCAPE_TODO(_format, ...) \
    do { \
        fprintf(stderr, "%s:%d: [TODO]: ", __FILE__, __LINE__); \
        fprintf(stderr, _format ESCAPE_COMMA_VA_ARGS(__VA_ARGS__)); \
        abort(); \
    } while (0)

/* ABOUT `SIMPLE, COMMON` */

#define ESCAPE_UNSAFE_MIN(a, b)         ((a) < (b) ? (a) : (b))
#define ESCAPE_SAFE_MIN(a, b)           ({ ESCAPE_TYPEOF(a) _a = (a); ESCAPE_TYPEOF(b) _b = (b); _a < _b ? _a : _b; })
#define ESCAPE_MIN ESCAPE_UNSAFE_MIN

#define ESCAPE_UNSAFE_MAX(a, b)         ((a) > (b) ? (a) : (b))
#define ESCAPE_SAFE_MAX(a, b)           ({ ESCAPE_TYPEOF(a) _a = (a); ESCAPE_TYPEOF(b) _b = (b); _a > _b ? _a : _b; })
#define ESCAPE_MAX ESCAPE_UNSAFE_MAX

#define ESCAPE_UNSAFE_CLAMP(x, l, h)    ((x) < (l) ? (l) : ((x) > (h) ? (h) : (x)))
#define ESCAPE_SAFE_CLAMP(x, l, h)      ({ ESCAPE_TYPEOF(x) _x = (x); ESCAPE_TYPEOF(l) _l = (l); ESCAPE_TYPEOF(h) _h = (h); _x < _l ? _l : (_x > _h ? _h : _x); })
#define ESCAPE_CLAMP ESCAPE_UNSAFE_CLAMP

#define ESCAPE_UNSAFE_IN_RANGE(x, l, h) (((x) >= (l)) && ((x) <= (h)))
#define ESCAPE_SAFE_IN_RANGE(x, l, h)   ({ ESCAPE_TYPEOF(x) _x = (x); ESCAPE_TYPEOF(l) _l = (l); ESCAPE_TYPEOF(h) _h = (h); (_x >= _l) && (_x <= _h); })
#define ESCAPE_IN_RANGE ESCAPE_UNSAFE_IN_RANGE

#define ESCAPE_UNSAFE_ROUND_DIV(a, b)   ((a) >= 0 ? (((a) + (b) / 2) / (b)) : (((a) - (b) / 2) / (b)))
#define ESCAPE_SAFE_ROUND_DIV(a, b)     ({ ESCAPE_TYPEOF(a) _a = (a); ESCAPE_TYPEOF(b) _b = (b); _a >= 0 ? ((_a + _b / 2) / _b) : ((_a - _b / 2) / _b); })
#define ESCAPE_ROUND_DIV ESCAPE_UNSAFE_ROUND_DIV

#pragma endregion "ESCAPE UTIL"



#pragma region "ESCAPE SEQUENCE TYPE (EST)"

#define EST_BLOCK_OFFSET 0x0100

#define ESCAPE_RGB(R, G, B)     R ";" G ";" B
#define ESCAPE_SRGB(R, G, B)    ESCAPE_S(R) ";" ESCAPE_S(G) ";" ESCAPE_S(B)

#define ESCAPE_JOIN_SEMICOLON_I(...)    ESCAPE_JOIN(__VA_ARGS__)
#define ESCAPE_JOIN_SEMICOLON(...)      ESCAPE_JOIN_SEMICOLON_I(";" ESCAPE_COMMA_VA_ARGS(__VA_ARGS__))
#define ESCAPE_JOINS_SEMICOLON_I(...)   ESCAPE_JOINS(__VA_ARGS__)
#define ESCAPE_JOINS_SEMICOLON(...)     ESCAPE_JOINS_SEMICOLON_I(";" ESCAPE_COMMA_VA_ARGS(__VA_ARGS__))

enum ESCAPE_SEQUENCE_TYPE_ORDER
{
    ESTO_BEGIN,

    ESTO_ESC,
    ESTO_CSI,
    ESTO_OSC,
    ESTO_DCS,
    ESTO_APC,
    ESTO_ST,
    ESTO_SOS,
    ESTO_PM,

    ESTO_END,
    ESTO_NUMS = (ESTO_END - 1 - ESTO_BEGIN),
};

enum ESCAPE_SEQUENCE_TYPE
{
    EST_BEGIN,

    EST_ESC = ESTO_ESC * EST_BLOCK_OFFSET,
    EST_CSI = ESTO_CSI * EST_BLOCK_OFFSET,
    EST_OSC = ESTO_OSC * EST_BLOCK_OFFSET,
    EST_DCS = ESTO_DCS * EST_BLOCK_OFFSET,
    EST_APC = ESTO_APC * EST_BLOCK_OFFSET,
    EST_ST  = ESTO_ST  * EST_BLOCK_OFFSET,
    EST_SOS = ESTO_SOS * EST_BLOCK_OFFSET,
    EST_PM  = ESTO_PM  * EST_BLOCK_OFFSET,

    EST_END,
    EST_NUMS = ESTO_NUMS
};

#pragma endregion "ESCAPE SEQUENCE TYPE (EST)"



#pragma region "ESCAPE BASIC"

#define ESC_VAL_H       0x1b
#define ESC_RAW_H       \x1b
#define ESC_CHR_H       '\x1b'
#define ESC_STR_H       "\x1b"

#define ESC_VAL_O       033
#define ESC_RAW_O       \033
#define ESC_CHR_O       '\033'
#define ESC_STR_O       "\033"

#define ESC_VAL_DEF     ESC_VAL_H
#define ESC_RAW_DEF     ESC_RAW_H
#define ESC_CHR_DEF     ESC_CHR_H
#define ESC_STR_DEF     ESC_STR_H

#define ESC_ESC(_)      ESC_STR_DEF _

/* Escape */
#define ESCAPE_INTRODUCER_ESC ESC_ESC("")
/* Control Sequence Introducer */
#define ESCAPE_INTRODUCER_CSI ESC_ESC("[")
/* Operating System Command */
#define ESCAPE_INTRODUCER_OSC ESC_ESC("]")
/* Device Control String */
#define ESCAPE_INTRODUCER_DCS ESC_ESC("P")
/* Application Program Command */
#define ESCAPE_INTRODUCER_APC ESC_ESC("_")
/* String Terminal */
#define ESCAPE_INTRODUCER_ST  ESC_ESC("\\")
/* Start of String */
#define ESCAPE_INTRODUCER_SOS ESC_ESC("X")
/* Privacy Message */
#define ESCAPE_INTRODUCER_PM  ESC_ESC("^")

/* Escape */
#define EST_ESC_STR(_I, F)          ESCAPE_INTRODUCER_ESC _I F
#define EST_ESC_NOI_STR(F)          EST_ESC_STR("", F)
/* Control Sequence Introducer */
#define EST_CSI_STR(_P, _I, F)      ESCAPE_INTRODUCER_CSI _P _I F
#define EST_CSI_NOP_STR(_I, F)      EST_CSI_STR("", _I, F)
#define EST_CSI_NOI_STR(_P, F)      EST_CSI_STR(_P, "", F)
#define EST_CSI_NOPI_STR(F)         EST_CSI_STR("", "", F)
/* Operating System Command */
#define EST_OSC_STR(S)              ESCAPE_INTRODUCER_OSC S ESCAPE_INTRODUCER_ST
/* Device Control String */
#define EST_DCS_STR(_P, _I, F, D)   ESCAPE_INTRODUCER_DCS _P _I F D ESCAPE_INTRODUCER_ST
#define EST_DCS_NOP_STR(_I, F)      EST_DCS_STR("", _I, F)
#define EST_DCS_NOI_STR(_P, F)      EST_DCS_STR(_P, "", F)
#define EST_DCS_NOPI_STR(F)         EST_DCS_STR("", "", F)
/* Application Program Command */
#define EST_APC_STR(D)              ESCAPE_INTRODUCER_APC D ESCAPE_INTRODUCER_ST
/* String Terminal */
#define EST_ST_STR()                ESCAPE_INTRODUCER_ST
/* Start of String */
#define EST_SOS_STR(D)              ESCAPE_INTRODUCER_SOS D ESCAPE_INTRODUCER_ST
/* Privacy Message */
#define EST_PM_STR(D)               ESCAPE_INTRODUCER_PM D ESCAPE_INTRODUCER_ST

#pragma endregion "ESCAPE BASIC"



#pragma region "EST CSI"

#pragma region "EST CSI TYPE"

enum EST_CSI_TYPE
{
    /* EST CSI BEGIN */

    EST_CSI_BEGIN = EST_CSI,

    /* CURSOR MOVING */

    EST_CSI_CUU,        /* Cursor Up */
    EST_CSI_CUD,        /* Cursor Down */
    EST_CSI_CUF,        /* Cursor Forward */
    EST_CSI_CUB,        /* Cursor Backward */
    EST_CSI_CNL,        /* Cursor Next Line */
    EST_CSI_CPL,        /* Cursor Preceding Line */
    EST_CSI_CHA,        /* Cursor Character Absolute */
    EST_CSI_CUP,        /* Cursor Position */
    EST_CSI_HVP,        /* Horizontal Vertical Position */
    EST_CSI_VPA,        /* Vertical Position Absolute */
    EST_CSI_CHT,        /* Cursor Forward Tabulation */
    EST_CSI_CBT,        /* Cursor Backward Tabulation */
    EST_CSI_SCP,        /* Save Cursor Position */
    EST_CSI_RCP,        /* Restore Cursor Position */
    EST_CSI_DECSC,      /* Save Cursor */
    EST_CSI_DECRC,      /* Restore Cursor */

    /* ERASE AND EDIT */

    EST_CSI_ED,         /* Erase in Display */
    EST_CSI_EL,         /* Erase in Line */
    EST_CSI_ICH,        /* Insert Blank Characters */
    EST_CSI_DCH,        /* Delete Characters */
    EST_CSI_ECH,        /* Erase Characters */
    EST_CSI_IL,         /* Insert Lines */
    EST_CSI_DL,         /* Delete Lines */
    EST_CSI_SU,         /* Scroll Up */
    EST_CSI_SD,         /* Scroll Down */
    EST_CSI_REP,        /* Repeat */

    /* TEXT DISPLAY */

    EST_CSI_SGR,        /* Select Graphic Rendition */

    /* SET / RESET MODE */

    EST_CSI_SM,         /* Set Mode */
    EST_CSI_RM,         /* Reset Mode */

    /* DEVICE STATUS REPORT */

    EST_CSI_DSR,        /* Device Status Report */
    EST_CSI_CPR,        /* Cursor Position Report */
    EST_CSI_DA1,        /* Primary Device Attributes */
    EST_CSI_DA2,        /* Secondary Device Attributes */
    EST_CSI_XTVERSION,  /* XTerminal Version */

    /* SCROLL REGION AND MARGIN */

    EST_CSI_DECSTBM,    /* Set Top and Bottom Margins */
    EST_CSI_DECSLRM,    /* Set Left and Right Margins */

    /* TAB(TABULATION) */

    EST_CSI_TBC,        /* Tab Clear */
    // EST_CSI_CHT,        /* Cursor Forward Tabulation */
    // EST_CSI_CBT,        /* Cursor Backward Tabulation */

    /* WINDOW OPERATION */

    /* OTHERS */

    EST_CSI_DECSTR,     /* Soft Terminal Reset */
    EST_CSI_DECSCUSR,   /* Set Cursor Style */
    EST_CSI_DECSCA,     /* Set Character Attribute */
    EST_CSI_DECRQM,     /* Request Mode */

    /* EST CSI END */

    EST_CSI_END,
    
    /* NUMBER OF CSI SUPPORTED */

    EST_CSI_NUMS = (EST_CSI_END - 1 - EST_CSI_BEGIN)
};

#pragma endregion "EST CSI TYPE"

#pragma region "EST CSI SGR PARAMS"

/* SGR COMMON ATTRIBUTES */

#define EST_CSI_SGR_RESET                       0
#define EST_CSI_SGR_NORMAL                      0
#define EST_CSI_SGR_BOLD                        1
#define EST_CSI_SGR_INCREASED_INTENSITY         1
#define EST_CSI_SGR_FAINT                       2
#define EST_CSI_SGR_DECREASED_INTENSITY         2
#define EST_CSI_SGR_ITALIC                      3
#define EST_CSI_SGR_UNDERLINE                   4
#define EST_CSI_SGR_SLOW_BLINK                  5
#define EST_CSI_SGR_RAPID_BLINK                 6
#define EST_CSI_SGR_REVERSE_VIDEO               7
#define EST_CSI_SGR_SWAP_FORE_BACK              7
#define EST_CSI_SGR_HIDE                        8
#define EST_CSI_SGR_CONCEAL                     8
#define EST_CSI_SGR_STRIKE                      9
#define EST_CSI_SGR_CROSSED_OUT                 9

#define EST_CSI_SGR_DOUBLY_UNDERLINE            21
#define EST_CSI_SGR_NOT_BOLD                    22
#define EST_CSI_SGR_NORMAL_INTENSITY            22
#define EST_CSI_SGR_NOT_ITALIC                  23
#define EST_CSI_SGR_NOT_UNDERLINE               24
#define EST_CSI_SGR_NOT_BLINKING                25
#define EST_CSI_SGR_PROPORTIONAL_SPACING        26
#define EST_CSI_SGR_NOT_REVERSED                27
#define EST_CSI_SGR_REVEAL                      28
#define EST_CSI_SGR_NOT_HIDDEN                  28
#define EST_CSI_SGR_NOT_STRIKE                  29
#define EST_CSI_SGR_NOT_CROSSED_OUT             29

/* SGR COLOR */

#define EST_CSI_SGR_FC_BLACK                    30
#define EST_CSI_SGR_FC_RED                      31
#define EST_CSI_SGR_FC_GREEN                    32
#define EST_CSI_SGR_FC_YELLOW                   33
#define EST_CSI_SGR_FC_BLUE                     34
#define EST_CSI_SGR_FC_MAGENTA                  35
#define EST_CSI_SGR_FC_CYAN                     36
#define EST_CSI_SGR_FC_WHITE                    37
#define EST_CSI_SGR_FC_EXT                      38
#define EST_CSI_SGR_FC_DEFAULT                  39

#define EST_CSI_SGR_BC_BLACK                    40
#define EST_CSI_SGR_BC_RED                      41
#define EST_CSI_SGR_BC_GREEN                    42
#define EST_CSI_SGR_BC_YELLOW                   43
#define EST_CSI_SGR_BC_BLUE                     44
#define EST_CSI_SGR_BC_MAGENTA                  45
#define EST_CSI_SGR_BC_CYAN                     46
#define EST_CSI_SGR_BC_WHITE                    47
#define EST_CSI_SGR_BC_EXT                      48
#define EST_CSI_SGR_BC_DEFAULT                  49

#define EST_CSI_SGR_NOT_PROPORTIONAL_SPACING    50

#define EST_CSI_SGR_UC_EXT                      58
#define EST_CSI_SGR_UC_DEFAULT                  59

#define EST_CSI_SGR_BFC_BLACK                   90
#define EST_CSI_SGR_BFC_RED                     91
#define EST_CSI_SGR_BFC_GREEN                   92
#define EST_CSI_SGR_BFC_YELLOW                  93
#define EST_CSI_SGR_BFC_BLUE                    94
#define EST_CSI_SGR_BFC_MAGENTA                 95
#define EST_CSI_SGR_BFC_CYAN                    96
#define EST_CSI_SGR_BFC_WHITE                   97

#define EST_CSI_SGR_BBC_BLACK                   100
#define EST_CSI_SGR_BBC_RED                     101
#define EST_CSI_SGR_BBC_GREEN                   102
#define EST_CSI_SGR_BBC_YELLOW                  103
#define EST_CSI_SGR_BBC_BLUE                    104
#define EST_CSI_SGR_BBC_MAGENTA                 105
#define EST_CSI_SGR_BBC_CYAN                    106
#define EST_CSI_SGR_BBC_WHITE                   107

#pragma endregion "EST CSI SGR PARAMS"

#pragma region "EST CSI SGR 256 COLORS"

/* STANDARD ANSI COLORS (BY TERMINAL THEME) */

#define EST_CSI_SGR_256_COLOR_BLACK     0
#define EST_CSI_SGR_256_COLOR_RED       1
#define EST_CSI_SGR_256_COLOR_GREEN     2
#define EST_CSI_SGR_256_COLOR_YELLOW    3
#define EST_CSI_SGR_256_COLOR_BLUE      4
#define EST_CSI_SGR_256_COLOR_MAGENTA   5
#define EST_CSI_SGR_256_COLOR_CYAN      6
#define EST_CSI_SGR_256_COLOR_WHITE     7

/* STANDARD ANSI BRIGHT COLORS (BY TERMINAL THEME) */

#define EST_CSI_SGR_256_COLOR_BBLACK    8
#define EST_CSI_SGR_256_COLOR_BRED      9
#define EST_CSI_SGR_256_COLOR_BGREEN    10
#define EST_CSI_SGR_256_COLOR_BYELLOW   11
#define EST_CSI_SGR_256_COLOR_BBLUE     12
#define EST_CSI_SGR_256_COLOR_BMAGENTA  13
#define EST_CSI_SGR_256_COLOR_BCYAN     14
#define EST_CSI_SGR_256_COLOR_BWHITE    15

/* 6*6*6 COLOR CUBE */

/* r/g/b LEVEL MAP */
static const int EST_CSI_SGR_256_COLOR_666_COLOR_CUBE_LEVEL[] = {0, 95, 135, 175, 215, 255};

#define EST_CSI_SGR_256_COLOR_CUBE_INDEX(level_r, level_g, level_b) (16 + 36 * (level_r) + 6 * (level_g) + (level_b))
#define EST_CSI_SGR_256_COLOR_CUBE_LEVEL_R(index)   (((index) - 16) / 36)
#define EST_CSI_SGR_256_COLOR_CUBE_LEVEL_G(index)   ((((index) - 16) % 36) / 6)
#define EST_CSI_SGR_256_COLOR_CUBE_LEVEL_B(index)   (((index) - 16) % 6)
#define EST_CSI_SGR_256_COLOR_CUBE_R(level_r)       EST_CSI_SGR_256_COLOR_666_COLOR_CUBE_LEVEL[(level_r)]
#define EST_CSI_SGR_256_COLOR_CUBE_G(level_g)       EST_CSI_SGR_256_COLOR_666_COLOR_CUBE_LEVEL[(level_g)]
#define EST_CSI_SGR_256_COLOR_CUBE_B(level_b)       EST_CSI_SGR_256_COLOR_666_COLOR_CUBE_LEVEL[(level_b)]

#define EST_CSI_SGR_256_COLOR_CUBE_MIN_LEVEL 0
#define EST_CSI_SGR_256_COLOR_CUBE_MAX_LEVEL 5
#define EST_CSI_SGR_256_COLOR_CUBE_MIN_INDEX 16
#define EST_CSI_SGR_256_COLOR_CUBE_MAX_INDEX 231
#define EST_CSI_SGR_256_COLOR_CUBE_LEVEL_IN_RANGE(level) ESCAPE_IN_RANGE(level, EST_CSI_SGR_256_COLOR_CUBE_MIN_LEVEL, EST_CSI_SGR_256_COLOR_CUBE_MAX_LEVEL)
#define EST_CSI_SGR_256_COLOR_CUBE_INDEX_IN_RANGE(index) ESCAPE_IN_RANGE(index, EST_CSI_SGR_256_COLOR_CUBE_MIN_INDEX, EST_CSI_SGR_256_COLOR_CUBE_MAX_INDEX)

/* 24-LEVEL GRAYSCALE */

#define EST_CSI_SGR_256_COLOR_GRAYSCALE_LEVEL(v)    ESCAPE_ROUND_DIV((v) - 8, 10)
#define EST_CSI_SGR_256_COLOR_GRAYSCALE_INDEX(v)    (232 + EST_CSI_SGR_256_COLOR_GRAYSCALE_LEVEL(v))
#define EST_CSI_SGR_256_COLOR_GRAYSCALE_GRAY(index) (8 + 10 * ((index) - 232))
#define EST_CSI_SGR_256_COLOR_GRAYSCALE_R(index)    EST_CSI_SGR_256_COLOR_GRAYSCALE_GRAY(index)
#define EST_CSI_SGR_256_COLOR_GRAYSCALE_G(index)    EST_CSI_SGR_256_COLOR_GRAYSCALE_GRAY(index)
#define EST_CSI_SGR_256_COLOR_GRAYSCALE_B(index)    EST_CSI_SGR_256_COLOR_GRAYSCALE_GRAY(index)

#define EST_CSI_SGR_256_COLOR_GRAYSCALE_MIN_I       0
#define EST_CSI_SGR_256_COLOR_GRAYSCALE_MAX_I       23
#define EST_CSI_SGR_256_COLOR_GRAYSCALE_MIN_INDEX   232
#define EST_CSI_SGR_256_COLOR_GRAYSCALE_MAX_INDEX   255
#define EST_CSI_SGR_256_COLOR_GRAYSCALE_I_IN_RANGE(i)           ESCAPE_IN_RANGE(i, EST_CSI_SGR_256_COLOR_GRAYSCALE_MIN_I, EST_CSI_SGR_256_COLOR_GRAYSCALE_MAX_I)
#define EST_CSI_SGR_256_COLOR_GRAYSCALE_INDEX_IN_RANGE(index)   ESCAPE_IN_RANGE(index, EST_CSI_SGR_256_COLOR_GRAYSCALE_MIN_INDEX, EST_CSI_SGR_256_COLOR_GRAYSCALE_MAX_INDEX)

#pragma endregion "EST CSI SGR 256 COLORS"

#pragma region "EST CSI SM/RM PARAMS"

/* ANSI STANDARD MODE */

#define EST_CSI_SM_RM_IRM       4
#define EST_CSI_SM_RM_LNM       20

/* DEC PRIVATE MODE */

#define EST_CSI_SM_RM_DECCKM    1
#define EST_CSI_SM_RM_DECCOLM   3
#define EST_CSI_SM_RM_DECSCNM   5
#define EST_CSI_SM_RM_DECOM     6
#define EST_CSI_SM_RM_DECAWM    7
#define EST_CSI_SM_RM_DECTCEM   25
#define EST_CSI_SM_RM_MODE_47   47
#define EST_CSI_SM_RM_DECLRMM   69
#define EST_CSI_SM_RM_MODE_1000 1000
#define EST_CSI_SM_RM_MODE_1002 1002
#define EST_CSI_SM_RM_MODE_1003 1003
#define EST_CSI_SM_RM_MODE_1004 1004
#define EST_CSI_SM_RM_MODE_1006 1006
#define EST_CSI_SM_RM_MODE_1047 1047
#define EST_CSI_SM_RM_MODE_1048 1048
#define EST_CSI_SM_RM_MODE_1049 1049
#define EST_CSI_SM_RM_MODE_2004 2004
#define EST_CSI_SM_RM_MODE_2026 2026

#pragma endregion "EST CSI SM/RM PARAMS"

#pragma region "EST CSI FINAL STRING"

#define EST_CSI_CUU_FS          "A"
#define EST_CSI_CUD_FS          "B"
#define EST_CSI_CUF_FS          "C"
#define EST_CSI_CUB_FS          "D"
#define EST_CSI_CNL_FS          "E"
#define EST_CSI_CPL_FS          "F"
#define EST_CSI_CHA_FS          "G"
#define EST_CSI_CUP_FS          "H"
#define EST_CSI_HVP_FS          "f"
#define EST_CSI_VPA_FS          "d"
#define EST_CSI_CHT_FS          "I"
#define EST_CSI_CBT_FS          "Z"
#define EST_CSI_SCP_FS          "s"
#define EST_CSI_RCP_FS          "u"
#define EST_CSI_DECSC_FS        "7"
#define EST_CSI_DECRC_FS        "8"

#define EST_CSI_ED_FS           "J"
#define EST_CSI_EL_FS           "K"
#define EST_CSI_ICH_FS          "@"
#define EST_CSI_DCH_FS          "P"
#define EST_CSI_ECH_FS          "X"
#define EST_CSI_IL_FS           "L"
#define EST_CSI_DL_FS           "M"
#define EST_CSI_SU_FS           "S"
#define EST_CSI_SD_FS           "T"
#define EST_CSI_REP_FS          "b"

#define EST_CSI_SGR_FS          "m"

#define EST_CSI_SM_FS           "h"
#define EST_CSI_RM_FS           "l"

#define EST_CSI_DSR_FS          "n"
#define EST_CSI_CPR_FS          "n"
#define EST_CSI_DA1_FS          "c"
#define EST_CSI_DA2_FS          "c"
#define EST_CSI_XTVERSION_FS    "q"

#define EST_CSI_DECSTBM_FS      "r"
#define EST_CSI_DECSLRM_FS      "s"

#define EST_CSI_TBC_FS          "g"
// #define EST_CSI_CHT_FS          "I"
// #define EST_CSI_CBT_FS          "Z"

#define EST_CSI_DECSTR_FS       "p"
#define EST_CSI_DECSCUSR_FS     "q"
#define EST_CSI_DECSCA_FS       "q"
#define EST_CSI_DECRQM_FS       "p"

#pragma endregion "EST CSI FINAL STRING"

#pragma region "EST CSI STRING"

#define EST_CSI_CUU_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_CUU_FS)
#define EST_CSI_CUD_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_CUD_FS)
#define EST_CSI_CUF_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_CUF_FS)
#define EST_CSI_CUB_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_CUB_FS)
#define EST_CSI_CNL_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_CNL_FS)
#define EST_CSI_CPL_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_CPL_FS)
#define EST_CSI_CHA_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_CHA_FS)
#define EST_CSI_CUP_STR(Pr, Pc)     EST_CSI_NOI_STR(ESCAPE_JOIN_SEMICOLON(Pr, Pc), EST_CSI_CUP_FS)
#define EST_CSI_HVP_STR(Pr, Pc)     EST_CSI_NOI_STR(ESCAPE_JOIN_SEMICOLON(Pr, Pc), EST_CSI_HVP_FS)
#define EST_CSI_VPA_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_VPA_FS)
#define EST_CSI_CHT_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_CHT_FS)
#define EST_CSI_CBT_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_CBT_FS)
/**
 * When it is disabled by default or set to `?69`: Save cursor position (SCP).
 * After enabling the left and right margin mode (DECLRMM) with `?69h`: it is interpreted as DECSLRM, which sets the left and right margins.
 */
#define EST_CSI_SCP_STR()           EST_CSI_NOPI_STR(EST_CSI_SCP_FS)
/**
 * Traditional: Restore cursor position (RCP).
 * Modern terminals: They may be occupied by extensions such as the Kitty keyboard protocol, and the specific implementation depends on the terminal.
 */
#define EST_CSI_RCP_STR()           EST_CSI_NOPI_STR(EST_CSI_RCP_FS)
#define EST_CSI_DECSC_STR()         EST_ESC_NOI_STR(EST_CSI_DECSC_FS)
#define EST_CSI_DECRC_STR()         EST_ESC_NOI_STR(EST_CSI_DECRC_FS)

/**
 * Ps: Used to distinguish sub-functions.
 * - 0: Erase from the cursor position to the end of the screen, including the cursor position (default).
 * - 1: Erase from the beginning of the screen to the cursor position, including the cursor position.
 * - 2: Erase the entire screen.
 * - 3: Erase scrollback buffer, an xterm extension, not supported by all terminals.
 */
#define EST_CSI_ED_STR(Ps)          EST_CSI_NOI_STR(Ps, EST_CSI_ED_FS)
/**
 * Ps: Used to distinguish sub-functions.
 * - 0: Erase from the cursor position to the end of the line, including the cursor position (default).
 * - 1: Erase from the beginning of the line to the cursor position, including the cursor.
 * - 2: Erase entire line.
 */
#define EST_CSI_EL_STR(Ps)          EST_CSI_NOI_STR(Ps, EST_CSI_EL_FS)
#define EST_CSI_ICH_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_ICH_FS)
#define EST_CSI_DCH_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_DCH_FS)
#define EST_CSI_ECH_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_ECH_FS)
#define EST_CSI_IL_STR(Ps)          EST_CSI_NOI_STR(Ps, EST_CSI_IL_FS)
#define EST_CSI_DL_STR(Ps)          EST_CSI_NOI_STR(Ps, EST_CSI_DL_FS)
#define EST_CSI_SU_STR(Ps)          EST_CSI_NOI_STR(Ps, EST_CSI_SU_FS)
#define EST_CSI_SD_STR(Ps)          EST_CSI_NOI_STR(Ps, EST_CSI_SD_FS)
#define EST_CSI_REP_STR(Ps)         EST_CSI_NOI_STR(Ps, EST_CSI_REP_FS)

#define EST_CSI_SGR_STR(Ps)                         EST_CSI_NOI_STR(Ps, EST_CSI_SGR_FS)
#define EST_CSI_SGR_COLOR_EXT_STR(m1, m2, c)        EST_CSI_SGR_STR(m1 ";" m2 ";" c)
#define EST_CSI_SGR_TRUE_COLOR_EXT_STR(m, r, g, b)  EST_CSI_SGR_COLOR_EXT_STR(m, "2", ESCAPE_RGB(r, g, b))
#define EST_CSI_SGR_256_COLOR_EXT_STR(m, n)         EST_CSI_SGR_COLOR_EXT_STR(m, "5", n)
#define EST_CSI_SGR_FC_EXT_TRUE_STR(r, g, b)        EST_CSI_SGR_TRUE_COLOR_EXT_STR(ESCAPE_S(EST_CSI_SGR_FC_EXT), r, g, b)
#define EST_CSI_SGR_BC_EXT_TRUE_STR(r, g, b)        EST_CSI_SGR_TRUE_COLOR_EXT_STR(ESCAPE_S(EST_CSI_SGR_BC_EXT), r, g, b)
#define EST_CSI_SGR_UC_EXT_TRUE_STR(r, g, b)        EST_CSI_SGR_TRUE_COLOR_EXT_STR(ESCAPE_S(EST_CSI_SGR_UC_EXT), r, g, b)
#define EST_CSI_SGR_FC_EXT_TRUE_SSTR(r, g, b)       EST_CSI_SGR_FC_EXT_TRUE_STR(ESCAPE_S(r), ESCAPE_S(g), ESCAPE_S(b))
#define EST_CSI_SGR_BC_EXT_TRUE_SSTR(r, g, b)       EST_CSI_SGR_BC_EXT_TRUE_STR(ESCAPE_S(r), ESCAPE_S(g), ESCAPE_S(b))
#define EST_CSI_SGR_UC_EXT_TRUE_SSTR(r, g, b)       EST_CSI_SGR_UC_EXT_TRUE_STR(ESCAPE_S(r), ESCAPE_S(g), ESCAPE_S(b))
#define EST_CSI_SGR_FC_EXT_256_STR(n)               EST_CSI_SGR_256_COLOR_EXT_STR(ESCAPE_S(EST_CSI_SGR_FC_EXT), n)
#define EST_CSI_SGR_BC_EXT_256_STR(n)               EST_CSI_SGR_256_COLOR_EXT_STR(ESCAPE_S(EST_CSI_SGR_BC_EXT), n)
#define EST_CSI_SGR_UC_EXT_256_STR(n)               EST_CSI_SGR_256_COLOR_EXT_STR(ESCAPE_S(EST_CSI_SGR_UC_EXT), n)
#define EST_CSI_SGR_FC_EXT_256_SSTR(n)              EST_CSI_SGR_FC_EXT_256_STR(ESCAPE_S(n))
#define EST_CSI_SGR_BC_EXT_256_SSTR(n)              EST_CSI_SGR_BC_EXT_256_STR(ESCAPE_S(n))
#define EST_CSI_SGR_UC_EXT_256_SSTR(n)              EST_CSI_SGR_UC_EXT_256_STR(ESCAPE_S(n))

#define EST_CSI_SM_STR(Ps)                      EST_CSI_NOI_STR(Ps, EST_CSI_SM_FS)
#define EST_CSI_SM_ANSI_STANDARD_MODE_STR(Ps)   EST_CSI_SM_STR("" Ps)
#define EST_CSI_SM_DEC_PRIVATE_MODE_STR(Ps)     EST_CSI_SM_STR("?" Ps)
#define EST_CSI_RM_STR(Ps)                      EST_CSI_NOI_STR(Ps, EST_CSI_RM_FS)
#define EST_CSI_RM_ANSI_STANDARD_MODE_STR(Ps)   EST_CSI_RM_STR("" Ps)
#define EST_CSI_RM_DEC_PRIVATE_MODE_STR(Ps)     EST_CSI_RM_STR("?" Ps)

#define EST_CSI_DSR_STR()       EST_CSI_NOI_STR("5", EST_CSI_DSR_FS)
#define EST_CSI_CPR_STR()       EST_CSI_NOI_STR("6", EST_CSI_CPR_FS)
#define EST_CSI_DA1_STR()       EST_CSI_NOI_STR("0", EST_CSI_DA1_FS)
#define EST_CSI_DA2_STR()       EST_CSI_NOI_STR(">0", EST_CSI_DA2_FS)
#define EST_CSI_XTVERSION_STR() EST_CSI_NOI_STR(">0", EST_CSI_XTVERSION_FS)

/**
 * Pt: The top row of the scrolling area, counting from 1.
 * - 1: (default).
 * Pb: The bottom row of the scrolling area, counting from 1.
 * - Bottom of the screen: (default).
 */
#define EST_CSI_DECSTBM_STR(Pt, Pb)     EST_CSI_NOI_STR(ESCAPE_JOIN_SEMICOLON(Pt, Pb), EST_CSI_DECSTBM_FS)
/**
 * Note: The left and right margin mode needs to be activated in advance.
 * Pl: The left column of the scrolling area, counting from 1.
 * - 1: (default).
 * Pr: The right column of the scrolling area, counting from 1.
 * - Right edge of the screen: (default).
 */
#define EST_CSI_DECSLRM_STR(Pl, Pr)     EST_CSI_NOI_STR(ESCAPE_JOIN_SEMICOLON(Pl, Pr), EST_CSI_DECSLRM_FS)

/**
 * Ps: Used to distinguish sub-functions.
 * - 0: Clear current (default).
 * - 3: Clear all.
 */
#define EST_CSI_TBC_STR(Ps) EST_CSI_NOI_STR(Ps, EST_CSI_TBC_FS)

#define EST_CSI_DECSTR_STR()                        EST_CSI_NOP_STR("!", EST_CSI_DECSTR_FS)
/**
 * Ps: Used to specify the shape and blinking state of the cursor.
 * - 0: Usually in the form of flickering blocks (default).
 * - 1: Block.
 * - 2: Stable Block.
 * - 3: Underline.
 * - 4: Stable Underline.
 * - 5: Vertical Line.
 * - 6: Stable Vertical Line.
 */
#define EST_CSI_DECSCUSR_STR(Ps)                    EST_CSI_STR(Ps, " ", EST_CSI_DECSCUSR_FS)
/**
 * Ps: Used to define whether subsequent characters can be erased by selective erase commands (DECSED, DECSEL, DECSERA).
 * - 0: Erasable (default).
 * - 1: Protected.
 * - 2: Equivalent to 0, erasable.
 */
#define EST_CSI_DECSCA_STR(Ps)                      EST_CSI_STR(Ps, "\"", EST_CSI_DECSCA_FS)
#define EST_CSI_DECRQM_STR(Ps)                      EST_CSI_STR(Ps, "$", EST_CSI_DECRQM_FS)
#define EST_CSI_DECRQM_ANSI_STANDARD_MODE_STR(Ps)   EST_CSI_DECRQM_STR("" Ps)
#define EST_CSI_DECRQM_DEC_PRIVATE_MODE_STR(Ps)     EST_CSI_DECRQM_STR("?" Ps)

#pragma region "EST CSI DEFAULT STRING"

#define EST_CSI_CUU_DEFAULT_STR()       EST_CSI_CUU_STR("1")
#define EST_CSI_CUD_DEFAULT_STR()       EST_CSI_CUD_STR("1")
#define EST_CSI_CUF_DEFAULT_STR()       EST_CSI_CUF_STR("1")
#define EST_CSI_CUB_DEFAULT_STR()       EST_CSI_CUB_STR("1")
#define EST_CSI_CNL_DEFAULT_STR()       EST_CSI_CNL_STR("1")
#define EST_CSI_CPL_DEFAULT_STR()       EST_CSI_CPL_STR("1")
#define EST_CSI_CHA_DEFAULT_STR()       EST_CSI_CHA_STR("1")
#define EST_CSI_CUP_DEFAULT_STR()       EST_CSI_CUP_STR("1", "1")
#define EST_CSI_HVP_DEFAULT_STR()       EST_CSI_HVP_STR("1", "1")
#define EST_CSI_VPA_DEFAULT_STR()       EST_CSI_VPA_STR("1")
#define EST_CSI_CHT_DEFAULT_STR()       EST_CSI_CHT_STR("1")
#define EST_CSI_CBT_DEFAULT_STR()       EST_CSI_CBT_STR("1")

#define EST_CSI_ED_DEFAULT_STR()        EST_CSI_ED_STR("0")
#define EST_CSI_EL_DEFAULT_STR()        EST_CSI_EL_STR("0")
#define EST_CSI_ICH_DEFAULT_STR()       EST_CSI_ICH_STR("1")
#define EST_CSI_DCH_DEFAULT_STR()       EST_CSI_DCH_STR("1")
#define EST_CSI_ECH_DEFAULT_STR()       EST_CSI_ECH_STR("1")
#define EST_CSI_IL_DEFAULT_STR()        EST_CSI_IL_STR("1")
#define EST_CSI_DL_DEFAULT_STR()        EST_CSI_DL_STR("1")
#define EST_CSI_SU_DEFAULT_STR()        EST_CSI_SU_STR("1")
#define EST_CSI_SD_DEFAULT_STR()        EST_CSI_SD_STR("1")
#define EST_CSI_REP_DEFAULT_STR()       EST_CSI_REP_STR("1")

#define EST_CSI_TBC_DEFAULT_STR()       EST_CSI_TBC_STR("0")

#define EST_CSI_DECSCUSR_DEFAULT_STR()  EST_CSI_DECSCUSR_STR("0")
#define EST_CSI_DECSCA_DEFAULT_STR()    EST_CSI_DECSCA_STR("0")

#pragma endregion "EST CSI DEFAULT STRING"

#pragma region "EST CSI SSTR"

#define EST_CSI_CUU_SSTR(Ps)        EST_CSI_CUU_STR(ESCAPE_S(Ps))
#define EST_CSI_CUD_SSTR(Ps)        EST_CSI_CUD_STR(ESCAPE_S(Ps))
#define EST_CSI_CUF_SSTR(Ps)        EST_CSI_CUF_STR(ESCAPE_S(Ps))
#define EST_CSI_CUB_SSTR(Ps)        EST_CSI_CUB_STR(ESCAPE_S(Ps))
#define EST_CSI_CNL_SSTR(Ps)        EST_CSI_CNL_STR(ESCAPE_S(Ps))
#define EST_CSI_CPL_SSTR(Ps)        EST_CSI_CPL_STR(ESCAPE_S(Ps))
#define EST_CSI_CHA_SSTR(Ps)        EST_CSI_CHA_STR(ESCAPE_S(Ps))
#define EST_CSI_CUP_SSTR(Pr, Pc)    EST_CSI_CUP_STR(ESCAPE_S(Pr), ESCAPE_S(Pc))
#define EST_CSI_HVP_SSTR(Pr, Pc)    EST_CSI_HVP_STR(ESCAPE_S(Pr), ESCAPE_S(Pc))
#define EST_CSI_VPA_SSTR(Ps)        EST_CSI_VPA_STR(ESCAPE_S(Ps))
#define EST_CSI_CHT_SSTR(Ps)        EST_CSI_CHT_STR(ESCAPE_S(Ps))
#define EST_CSI_CBT_SSTR(Ps)        EST_CSI_CBT_STR(ESCAPE_S(Ps))
#define EST_CSI_SCP_SSTR()          EST_CSI_SCP_STR()
#define EST_CSI_RCP_SSTR()          EST_CSI_RCP_STR()
#define EST_CSI_DECSC_SSTR()        EST_CSI_DECSC_STR()
#define EST_CSI_DECRC_SSTR()        EST_CSI_DECRC_STR()
#define EST_CSI_ED_SSTR(Ps)         EST_CSI_ED_STR(ESCAPE_S(Ps))
#define EST_CSI_EL_SSTR(Ps)         EST_CSI_EL_STR(ESCAPE_S(Ps))
#define EST_CSI_ICH_SSTR(Ps)        EST_CSI_ICH_STR(ESCAPE_S(Ps))
#define EST_CSI_DCH_SSTR(Ps)        EST_CSI_DCH_STR(ESCAPE_S(Ps))
#define EST_CSI_ECH_SSTR(Ps)        EST_CSI_ECH_STR(ESCAPE_S(Ps))
#define EST_CSI_IL_SSTR(Ps)         EST_CSI_IL_STR(ESCAPE_S(Ps))
#define EST_CSI_DL_SSTR(Ps)         EST_CSI_DL_STR(ESCAPE_S(Ps))
#define EST_CSI_SU_SSTR(Ps)         EST_CSI_SU_STR(ESCAPE_S(Ps))
#define EST_CSI_SD_SSTR(Ps)         EST_CSI_SD_STR(ESCAPE_S(Ps))
#define EST_CSI_REP_SSTR(Ps)        EST_CSI_REP_STR(ESCAPE_S(Ps))
#define EST_CSI_SGR_SSTR(Ps)        EST_CSI_SGR_STR(ESCAPE_S(Ps))
#define EST_CSI_SM_SSTR(Ps)         EST_CSI_SM_STR(ESCAPE_S(Ps))
#define EST_CSI_RM_SSTR(Ps)         EST_CSI_RM_STR(ESCAPE_S(Ps))

#pragma endregion "EST CSI SSTR"

#pragma endregion "EST CSI STRING"

#pragma endregion "EST CSI"



#pragma region "ESCAPE FUNCTION"

#pragma region "ESCAPE COLOR"

#define ESCAPE_COLORIZE_STR(text, ...) \
    EST_CSI_SGR_STR(ESCAPE_JOIN_SEMICOLON(__VA_ARGS__)) text EST_CSI_SGR_STR(ESCAPE_S(EST_CSI_SGR_RESET))
#define ESCAPE_COLORIZE_SSTR(text, ...) \
    EST_CSI_SGR_STR(ESCAPE_JOINS_SEMICOLON(__VA_ARGS__)) text EST_CSI_SGR_STR(ESCAPE_S(EST_CSI_SGR_RESET))

#pragma endregion "ESCAPE COLOR"

#pragma endregion "ESCAPE FUNCTION"


#ifdef __cplusplus
}
#endif

#endif /* ESCAPE_H */
