#ifndef GBUILD_H
#define GBUILD_H

/* maximum number of source files that a project can contain */
#define MAX_PROJECT_FILES  16

/* mnemonic is a jump opcode using a relative code position */
#define MNE_RELJUMP        0x01
/* mnemonic is a floating point operation */
#define MNE_FLOAT          0x02
/* mnemonic is memory allocation opcode */
#define MNE_MALLOC         0x04
/* mnemonic memory resize opcode */
#define MNE_RESIZE         0x08

/*
Specifies the type of a token generated by the lexer.
*/
enum tokentype_t {
    UNKNOWN,
    IDENTIFIER,
    RESERVED,
    INTEGER,
    FLOAT,
    STRING,
    CHAR,
    DICT_WORD,
    OPEN_PARAN,
    CLOSE_PARAN,
    OPEN_BRACE,
    CLOSE_BRACE,
    COMMA,
    SEMICOLON
};

enum statement_type_t {
    STMT_UNKNOWN,
    STMT_BLOCK,
    STMT_ASM
};

/*
Stores information about a single assembly mnemonic.
*/
typedef struct ASM_MNEMONIC {
    const char *mnemonic;
    int opcode;
    int operands;
    int flags;
} mnemonic_t;

/*
Stores information about a project.
*/
typedef struct PROJECT {
    char *project_file;
    unsigned int switches;
    unsigned int file_count;
    char *files[MAX_PROJECT_FILES];
} project_t;

/*
Stores information about a lexer token.
*/
typedef struct LEXER_TOKEN {
    int type;
    const char *filename;
    int line_no;
    int col_no;

    union {
        const char *text;
        int integer;
    };

    struct LEXER_TOKEN *prev;
    struct LEXER_TOKEN *next;
} lexertoken_t;

/*
Stores a list of lexer tokens.
*/
typedef struct TOKEN_LIST {
    lexertoken_t *first;
    lexertoken_t *last;
} tokenlist_t;

/*
Store a block of assembly opcodes
*/
typedef struct ASMBLOCK_DEF {
    int a;
} asmblock_t;

/*
Stores a block of code
*/
struct STATEMENT_DEF;
typedef struct CODEBLOCK_DEF {
    struct STATEMENT_DEF *content;
} codeblock_t;

/*
Store an individual 'statement'
*/
typedef struct STATEMENT_DEF {
    int type;
    union {
        codeblock_t *code;
        asmblock_t *asm;
    };

    struct STATEMENT_DEF *prev;
    struct STATEMENT_DEF *next;
} statement_t;

/*
Store a function definition and associated code block.
*/
typedef struct FUNCTION_DEF {
    const char *name;
    codeblock_t *code;

    struct FUNCTION_DEF *prev;
    struct FUNCTION_DEF *next;
} function_t;

/*
Keep track of all data and content that makes up the content of a glulx game file.
*/
typedef struct GLULXFILE {
    void *constants;
    function_t *functions;
    void *globals;
    void *objects;
} glulxfile_t;

project_t* open_project(const char *project_file);
void free_project(project_t *project);

tokenlist_t* lex_file(const char *filename);
tokenlist_t* lex_string(const char *filename, const char *text, size_t length);
tokenlist_t* merge_tokens(tokenlist_t *first, tokenlist_t *second);
void free_tokens(tokenlist_t *tokens);

int parse_file(glulxfile_t *gamedata, tokenlist_t *tokens);

char *strdup (const char *source_string);
int is_reserved_word(const char *word);
mnemonic_t* get_mnemonic(const char *name);

void free_gamefile(glulxfile_t *what);
void free_function(function_t *what);
void free_codeblock(codeblock_t *what);
void free_asmblock(asmblock_t *what);

#endif
