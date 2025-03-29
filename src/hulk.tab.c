/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 1 "src\\hulk.y"

#include <stdio.h>
#include <stdlib.h>
#include "hulk.tab.h"
#include "ast.h"

/* Line 371 of yacc.c  */
#line 75 "src\\hulk.tab.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "hulk.tab.h".  */
#ifndef YY_YY_SRC_HULK_TAB_H_INCLUDED
# define YY_YY_SRC_HULK_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUMBER = 258,
     IDENTIFIER = 259,
     STRING_LIT = 260,
     FUNCTION = 261,
     LET = 262,
     IN = 263,
     IF = 264,
     ELIF = 265,
     ELSE = 266,
     TRUE = 267,
     FALSE = 268,
     PI = 269,
     E = 270,
     PLUS = 271,
     MINUS = 272,
     TIMES = 273,
     DIVIDE = 274,
     POWER = 275,
     XOR = 276,
     CONCAT = 277,
     DCONCAT = 278,
     EQ = 279,
     NEQ = 280,
     LT = 281,
     GT = 282,
     LEQ = 283,
     GEQ = 284,
     AND = 285,
     OR = 286,
     NOT = 287,
     ASSIGN = 288,
     ARROW = 289,
     SEMICOLON = 290,
     COMMA = 291,
     LPAREN = 292,
     RPAREN = 293,
     LBRACE = 294,
     RBRACE = 295
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 387 of yacc.c  */
#line 10 "src\\hulk.y"

    double num;
    char *str;
    struct ASTNode *node;


/* Line 387 of yacc.c  */
#line 165 "src\\hulk.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_SRC_HULK_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 205 "src\\hulk.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  40
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   163

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  29
/* YYNRULES -- Number of rules.  */
#define YYNRULES  70
/* YYNRULES -- Number of states.  */
#define YYNSTATES  142

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   295

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     6,     8,    10,    11,    14,    16,    18,
      20,    22,    26,    29,    30,    39,    46,    47,    52,    57,
      58,    64,    65,    68,    72,    73,    76,    80,    81,    84,
      88,    92,    96,   100,   104,   108,   109,   112,   116,   120,
     124,   128,   129,   132,   136,   140,   144,   148,   149,   151,
     154,   156,   160,   163,   165,   167,   169,   171,   174,   178,
     181,   182,   191,   198,   201,   202,   206,   207,   210,   211,
     215
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      42,     0,    -1,    65,    42,    -1,    44,    -1,    35,    -1,
      -1,    45,    43,    -1,    46,    -1,    48,    -1,    50,    -1,
      53,    -1,    39,    47,    40,    -1,    44,    47,    -1,    -1,
       9,    37,    45,    38,    44,    49,    11,    44,    -1,    10,
      37,    45,    38,    44,    49,    -1,    -1,     7,    51,     8,
      44,    -1,     4,    33,    45,    52,    -1,    -1,    36,     4,
      33,    45,    52,    -1,    -1,    55,    54,    -1,    31,    55,
      54,    -1,    -1,    57,    56,    -1,    30,    57,    56,    -1,
      -1,    59,    58,    -1,    24,    59,    58,    -1,    25,    59,
      58,    -1,    26,    59,    58,    -1,    27,    59,    58,    -1,
      28,    59,    58,    -1,    29,    59,    58,    -1,    -1,    61,
      60,    -1,    16,    61,    60,    -1,    17,    61,    60,    -1,
      22,    61,    60,    -1,    23,    61,    60,    -1,    -1,    63,
      62,    -1,    18,    63,    62,    -1,    19,    63,    62,    -1,
      20,    63,    62,    -1,    21,    63,    62,    -1,    -1,     3,
      -1,     4,    64,    -1,     5,    -1,    37,    45,    38,    -1,
      17,    63,    -1,    14,    -1,    15,    -1,    12,    -1,    13,
      -1,    32,    63,    -1,    37,    68,    38,    -1,    33,    45,
      -1,    -1,     6,     4,    37,    66,    38,    34,    45,    35,
      -1,     6,     4,    37,    66,    38,    46,    -1,     4,    67,
      -1,    -1,    36,     4,    67,    -1,    -1,    45,    69,    -1,
      -1,    36,    45,    69,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    33,    33,    34,    37,    37,    40,    41,    45,    46,
      47,    51,    55,    56,    60,    64,    65,    69,    73,    74,
      78,    79,    83,    87,    88,    92,    96,    97,   101,   105,
     106,   107,   108,   109,   110,   111,   115,   119,   120,   121,
     122,   123,   127,   131,   132,   133,   134,   135,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   152,   153,
     154,   158,   159,   163,   164,   168,   169,   173,   174,   178,
     179
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NUMBER", "IDENTIFIER", "STRING_LIT",
  "FUNCTION", "LET", "IN", "IF", "ELIF", "ELSE", "TRUE", "FALSE", "PI",
  "E", "PLUS", "MINUS", "TIMES", "DIVIDE", "POWER", "XOR", "CONCAT",
  "DCONCAT", "EQ", "NEQ", "LT", "GT", "LEQ", "GEQ", "AND", "OR", "NOT",
  "ASSIGN", "ARROW", "SEMICOLON", "COMMA", "LPAREN", "RPAREN", "LBRACE",
  "RBRACE", "$accept", "Program", "OpEnd", "P", "Exp", "Block", "ExpList",
  "IfExp", "ElifList", "LetIn", "AssignList", "MoreAssign", "LogicOr",
  "OpOr", "LogicAnd", "OpAnd", "Compare", "OpCompare", "X", "Op1", "F",
  "Op3", "T", "TRest", "FunctionDef", "ArgList", "MoreArgs", "ArgListCall",
  "MoreArgsCall", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    41,    42,    42,    43,    43,    44,    44,    45,    45,
      45,    46,    47,    47,    48,    49,    49,    50,    51,    51,
      52,    52,    53,    54,    54,    55,    56,    56,    57,    58,
      58,    58,    58,    58,    58,    58,    59,    60,    60,    60,
      60,    60,    61,    62,    62,    62,    62,    62,    63,    63,
      63,    63,    63,    63,    63,    63,    63,    63,    64,    64,
      64,    65,    65,    66,    66,    67,    67,    68,    68,    69,
      69
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     1,     0,     2,     1,     1,     1,
       1,     3,     2,     0,     8,     6,     0,     4,     4,     0,
       5,     0,     2,     3,     0,     2,     3,     0,     2,     3,
       3,     3,     3,     3,     3,     0,     2,     3,     3,     3,
       3,     0,     2,     3,     3,     3,     3,     0,     1,     2,
       1,     3,     2,     1,     1,     1,     1,     2,     3,     2,
       0,     8,     6,     2,     0,     3,     0,     2,     0,     3,
       0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    48,    60,    50,     0,    19,     0,    55,    56,    53,
      54,     0,     0,     0,    13,     0,     3,     5,     7,     8,
       9,    10,    24,    27,    35,    41,    47,     0,     0,    68,
      49,     0,     0,     0,     0,    52,    57,     0,    13,     0,
       1,     4,     6,     0,    22,     0,    25,     0,     0,     0,
       0,     0,     0,    28,     0,     0,     0,     0,    36,     0,
       0,     0,     0,    42,     2,    59,    70,     0,    64,     0,
       0,     0,    51,    12,    11,    24,    27,    35,    35,    35,
      35,    35,    35,    41,    41,    41,    41,    47,    47,    47,
      47,     0,    67,    58,    66,     0,    21,    17,     0,    23,
      26,    29,    30,    31,    32,    33,    34,    37,    38,    39,
      40,    43,    44,    45,    46,    70,     0,    63,     0,     0,
      18,    16,    69,    66,     0,    62,     0,     0,     0,    65,
       0,     0,     0,     0,    61,    21,     0,    14,    20,     0,
      16,    15
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    15,    42,    16,    17,    18,    39,    19,   128,    20,
      33,   120,    21,    44,    22,    46,    23,    53,    24,    58,
      25,    63,    26,    30,    27,    95,   117,    67,    92
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -37
static const yytype_int8 yypact[] =
{
      23,   -37,   -22,   -37,    -3,     4,   -28,   -37,   -37,   -37,
     -37,   126,   126,    91,    60,    14,   -37,   -19,   -37,   -37,
     -37,   -37,    -6,     3,    17,   -10,     1,    23,    91,    91,
     -37,     2,    -2,    26,    91,   -37,   -37,    19,    60,    21,
     -37,   -37,   -37,   126,   -37,   126,   -37,   126,   126,   126,
     126,   126,   126,   -37,   126,   126,   126,   126,   -37,   126,
     126,   126,   126,   -37,   -37,   -37,    30,    32,    64,    91,
      60,    33,   -37,   -37,   -37,    -6,     3,    17,    17,    17,
      17,    17,    17,   -10,   -10,   -10,   -10,     1,     1,     1,
       1,    91,   -37,   -37,    40,    41,    42,   -37,    60,   -37,
     -37,   -37,   -37,   -37,   -37,   -37,   -37,   -37,   -37,   -37,
     -37,   -37,   -37,   -37,   -37,    30,    77,   -37,   -29,    78,
     -37,    49,   -37,    40,    91,   -37,    50,    54,    82,   -37,
      66,    91,    91,    60,   -37,    42,    69,   -37,   -37,    60,
      49,   -37
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -37,    75,   -37,   -14,   -11,     0,    84,   -37,   -16,   -37,
     -37,    -9,   -37,    52,    99,    68,   100,     8,    85,   -36,
      55,    27,    -8,   -37,   -37,   -37,    24,   -37,    31
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      38,    31,    37,    35,    36,   124,    54,    55,    32,    34,
      14,    28,    56,    57,    40,    29,    41,    65,    66,    59,
      60,    61,    62,    71,    38,    43,     1,     2,     3,     4,
       5,    69,     6,    45,    70,     7,     8,     9,    10,    68,
      11,    47,    48,    49,    50,    51,    52,   107,   108,   109,
     110,    87,    88,    89,    90,    12,    97,    72,    96,   127,
      13,    74,    14,     1,     2,     3,    91,     5,    94,     6,
      93,    98,     7,     8,     9,    10,   116,    11,   119,   118,
     115,   123,   126,   131,   121,   101,   102,   103,   104,   105,
     106,   132,    12,   133,     1,     2,     3,    13,     5,    14,
       6,   134,    64,     7,     8,     9,    10,   139,    11,    83,
      84,    85,    86,   130,   111,   112,   113,   114,   125,   137,
     135,   136,    73,    12,   141,   140,   138,    99,    13,     1,
       2,     3,    77,    78,    79,    80,    81,    82,     7,     8,
       9,    10,    75,    11,   100,    76,   122,   129,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
       0,     0,     0,    13
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-37)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      14,     4,    13,    11,    12,    34,    16,    17,     4,    37,
      39,    33,    22,    23,     0,    37,    35,    28,    29,    18,
      19,    20,    21,    34,    38,    31,     3,     4,     5,     6,
       7,    33,     9,    30,     8,    12,    13,    14,    15,    37,
      17,    24,    25,    26,    27,    28,    29,    83,    84,    85,
      86,    59,    60,    61,    62,    32,    70,    38,    69,    10,
      37,    40,    39,     3,     4,     5,    36,     7,     4,     9,
      38,    38,    12,    13,    14,    15,    36,    17,    36,    38,
      91,     4,     4,    33,    98,    77,    78,    79,    80,    81,
      82,    37,    32,    11,     3,     4,     5,    37,     7,    39,
       9,    35,    27,    12,    13,    14,    15,    38,    17,    54,
      55,    56,    57,   124,    87,    88,    89,    90,   118,   133,
     131,   132,    38,    32,   140,   139,   135,    75,    37,     3,
       4,     5,    47,    48,    49,    50,    51,    52,    12,    13,
      14,    15,    43,    17,    76,    45,   115,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    37
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     9,    12,    13,    14,
      15,    17,    32,    37,    39,    42,    44,    45,    46,    48,
      50,    53,    55,    57,    59,    61,    63,    65,    33,    37,
      64,     4,     4,    51,    37,    63,    63,    45,    44,    47,
       0,    35,    43,    31,    54,    30,    56,    24,    25,    26,
      27,    28,    29,    58,    16,    17,    22,    23,    60,    18,
      19,    20,    21,    62,    42,    45,    45,    68,    37,    33,
       8,    45,    38,    47,    40,    55,    57,    59,    59,    59,
      59,    59,    59,    61,    61,    61,    61,    63,    63,    63,
      63,    36,    69,    38,     4,    66,    45,    44,    38,    54,
      56,    58,    58,    58,    58,    58,    58,    60,    60,    60,
      60,    62,    62,    62,    62,    45,    36,    67,    38,    36,
      52,    44,    69,     4,    34,    46,     4,    10,    49,    67,
      45,    33,    37,    11,    35,    45,    45,    44,    52,    38,
      44,    49
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (YYID (N))                                                     \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (YYID (0))
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if (! defined __GNUC__ || __GNUC__ < 2 \
      || (__GNUC__ == 2 && __GNUC_MINOR__ < 5))
#  define __attribute__(Spec) /* empty */
# endif
#endif

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

__attribute__((__unused__))
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
#else
static unsigned
yy_location_print_ (yyo, yylocp)
    FILE *yyo;
    YYLTYPE const * const yylocp;
#endif
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += fprintf (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += fprintf (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += fprintf (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += fprintf (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += fprintf (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}




/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
/* The lookahead symbol.  */
int yychar;


#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
static YYSTYPE yyval_default;
# define YY_INITIAL_VALUE(Value) = Value
#endif
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc = yyloc_default;


    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
/* Line 1792 of yacc.c  */
#line 33 "src\\hulk.y"
    { (yyval.node) = create_program_node((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); }
    break;

  case 3:
/* Line 1792 of yacc.c  */
#line 34 "src\\hulk.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 40 "src\\hulk.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); }
    break;

  case 7:
/* Line 1792 of yacc.c  */
#line 41 "src\\hulk.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 45 "src\\hulk.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 9:
/* Line 1792 of yacc.c  */
#line 46 "src\\hulk.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 10:
/* Line 1792 of yacc.c  */
#line 47 "src\\hulk.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 51 "src\\hulk.y"
    { (yyval.node) = create_block_node((yyvsp[(2) - (3)].node)); }
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 55 "src\\hulk.y"
    { (yyval.node) = create_exp_list_node((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); }
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 56 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 60 "src\\hulk.y"
    { (yyval.node) = create_if_node((yyvsp[(3) - (8)].node), (yyvsp[(5) - (8)].node), (yyvsp[(6) - (8)].node), (yyvsp[(8) - (8)].node)); }
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 64 "src\\hulk.y"
    { (yyval.node) = create_elif_node((yyvsp[(3) - (6)].node), (yyvsp[(5) - (6)].node), (yyvsp[(6) - (6)].node)); }
    break;

  case 16:
/* Line 1792 of yacc.c  */
#line 65 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 69 "src\\hulk.y"
    { (yyval.node) = create_let_in_node((yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node)); }
    break;

  case 18:
/* Line 1792 of yacc.c  */
#line 73 "src\\hulk.y"
    { (yyval.node) = create_assign_list_node((yyvsp[(1) - (4)].str), (yyvsp[(3) - (4)].node), (yyvsp[(4) - (4)].node)); }
    break;

  case 19:
/* Line 1792 of yacc.c  */
#line 74 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 78 "src\\hulk.y"
    { (yyval.node) = create_more_assign_node((yyvsp[(2) - (5)].str), (yyvsp[(4) - (5)].node), (yyvsp[(5) - (5)].node)); }
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 79 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 83 "src\\hulk.y"
    { (yyval.node) = create_logic_or_node((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); }
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 87 "src\\hulk.y"
    { (yyval.node) = create_op_or_node((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 88 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 92 "src\\hulk.y"
    { (yyval.node) = create_logic_and_node((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); }
    break;

  case 26:
/* Line 1792 of yacc.c  */
#line 96 "src\\hulk.y"
    { (yyval.node) = create_op_and_node((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 97 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 101 "src\\hulk.y"
    { (yyval.node) = create_compare_node((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); }
    break;

  case 29:
/* Line 1792 of yacc.c  */
#line 105 "src\\hulk.y"
    { (yyval.node) = create_op_compare_node(EQ, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 106 "src\\hulk.y"
    { (yyval.node) = create_op_compare_node(NEQ, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 31:
/* Line 1792 of yacc.c  */
#line 107 "src\\hulk.y"
    { (yyval.node) = create_op_compare_node(LT, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 108 "src\\hulk.y"
    { (yyval.node) = create_op_compare_node(GT, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 109 "src\\hulk.y"
    { (yyval.node) = create_op_compare_node(LEQ, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 110 "src\\hulk.y"
    { (yyval.node) = create_op_compare_node(GEQ, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 111 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 36:
/* Line 1792 of yacc.c  */
#line 115 "src\\hulk.y"
    { (yyval.node) = create_x_node((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); }
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 119 "src\\hulk.y"
    { (yyval.node) = create_op1_node(PLUS, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 120 "src\\hulk.y"
    { (yyval.node) = create_op1_node(MINUS, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 121 "src\\hulk.y"
    { (yyval.node) = create_op1_node(CONCAT, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 122 "src\\hulk.y"
    { (yyval.node) = create_op1_node(DCONCAT, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 123 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 127 "src\\hulk.y"
    { (yyval.node) = create_f_node((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); }
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 131 "src\\hulk.y"
    { (yyval.node) = create_op3_node(TIMES, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 44:
/* Line 1792 of yacc.c  */
#line 132 "src\\hulk.y"
    { (yyval.node) = create_op3_node(DIVIDE, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 133 "src\\hulk.y"
    { (yyval.node) = create_op3_node(POWER, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 46:
/* Line 1792 of yacc.c  */
#line 134 "src\\hulk.y"
    { (yyval.node) = create_op3_node(XOR, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 135 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 48:
/* Line 1792 of yacc.c  */
#line 139 "src\\hulk.y"
    { (yyval.node) = create_number_node((yyvsp[(1) - (1)].num)); }
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 140 "src\\hulk.y"
    { (yyval.node) = create_identifier_node((yyvsp[(1) - (2)].str), (yyvsp[(2) - (2)].node)); }
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 141 "src\\hulk.y"
    { (yyval.node) = create_string_node((yyvsp[(1) - (1)].str)); }
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 142 "src\\hulk.y"
    { (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 143 "src\\hulk.y"
    { (yyval.node) = create_unary_op_node(MINUS, (yyvsp[(2) - (2)].node)); }
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 144 "src\\hulk.y"
    { (yyval.node) = create_pi_node(); }
    break;

  case 54:
/* Line 1792 of yacc.c  */
#line 145 "src\\hulk.y"
    { (yyval.node) = create_e_node(); }
    break;

  case 55:
/* Line 1792 of yacc.c  */
#line 146 "src\\hulk.y"
    { (yyval.node) = create_boolean_node(1); }
    break;

  case 56:
/* Line 1792 of yacc.c  */
#line 147 "src\\hulk.y"
    { (yyval.node) = create_boolean_node(0); }
    break;

  case 57:
/* Line 1792 of yacc.c  */
#line 148 "src\\hulk.y"
    { (yyval.node) = create_unary_op_node(NOT, (yyvsp[(2) - (2)].node)); }
    break;

  case 58:
/* Line 1792 of yacc.c  */
#line 152 "src\\hulk.y"
    { (yyval.node) = create_function_call_node((yyvsp[(2) - (3)].node)); }
    break;

  case 59:
/* Line 1792 of yacc.c  */
#line 153 "src\\hulk.y"
    { (yyval.node) = create_assignment_node((yyvsp[(2) - (2)].node)); }
    break;

  case 60:
/* Line 1792 of yacc.c  */
#line 154 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 61:
/* Line 1792 of yacc.c  */
#line 158 "src\\hulk.y"
    { (yyval.node) = create_function_def_node((yyvsp[(2) - (8)].str), (yyvsp[(4) - (8)].node), (yyvsp[(7) - (8)].node)); }
    break;

  case 62:
/* Line 1792 of yacc.c  */
#line 159 "src\\hulk.y"
    { (yyval.node) = create_function_def_block_node((yyvsp[(2) - (6)].str), (yyvsp[(4) - (6)].node), (yyvsp[(6) - (6)].node)); }
    break;

  case 63:
/* Line 1792 of yacc.c  */
#line 163 "src\\hulk.y"
    { (yyval.node) = create_arg_list_node((yyvsp[(1) - (2)].str), (yyvsp[(2) - (2)].node)); }
    break;

  case 64:
/* Line 1792 of yacc.c  */
#line 164 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 65:
/* Line 1792 of yacc.c  */
#line 168 "src\\hulk.y"
    { (yyval.node) = create_more_args_node((yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].node)); }
    break;

  case 66:
/* Line 1792 of yacc.c  */
#line 169 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 67:
/* Line 1792 of yacc.c  */
#line 173 "src\\hulk.y"
    { (yyval.node) = create_arg_list_call_node((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); }
    break;

  case 68:
/* Line 1792 of yacc.c  */
#line 174 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;

  case 69:
/* Line 1792 of yacc.c  */
#line 178 "src\\hulk.y"
    { (yyval.node) = create_more_args_call_node((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 70:
/* Line 1792 of yacc.c  */
#line 179 "src\\hulk.y"
    { (yyval.node) = NULL; }
    break;


/* Line 1792 of yacc.c  */
#line 2056 "src\\hulk.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


/* Line 2055 of yacc.c  */
#line 182 "src\\hulk.y"


int main(int argc, char *argv[]) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "No se pudo abrir el archivo %s\n", argv[1]);
            return 1;
        }
    } else {
        yyin = stdin;
    }
    
    yyparse();
    return 0;
}