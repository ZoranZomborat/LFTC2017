void err(const char *fmt, ...);

typedef union TokenInfo{
        char *text; // folosit pentru ID, CT_STRING (alocat dinamic)
        long int intnum; // folosit pentru CT_INT, CT_CHAR
        double floatnum; // folosit pentru CT_REAL
}tokenInfo;

typedef enum{
    TK_NONE=0,
    TK_STRING,
    TK_INT,
    TK_FLOAT
}tokenInfoType;

typedef struct _Token{
	int code; // codul (numele)
	tokenInfo info;
	tokenInfoType infoType;
	int line; // linia din fisierul de intrare
	struct _Token *next; // inlantuire la urmatorul AL
}Token;

typedef enum{
    BREAK=0     ,
    CHAR        ,
    DOUBLE      ,
    ELSE        ,
    FOR         ,
    IF          ,
    INT         ,
    RETURN      ,
    STRUCT      ,
    VOID        ,
    WHILE       ,
    CT_INT      ,
    CT_REAL     ,
    ID          ,
    ESC         ,
    CT_CHAR     ,
    CT_STRING   ,
    COMMA       ,
    SEMICOL     ,
    LPAR        ,
    RPAR        ,    LBRACK      ,    RBRACK      ,    LACC        ,    RACC        ,    ADD         ,    SUB         ,    MUL         ,
    DOT         ,    DIV         ,    AND         ,    OR          ,    NOT         ,    ASSIGN      ,    EQUAL       ,    NEQUAL      ,    LESS        ,    LESSEQ      ,    GRT         ,    GRTEQ       ,
    END         ,
    NUM_ATOMS   ,
    FIRST_ATOM  = BREAK ,
    LAST_ATOM   = END,
    FIRST_KEYW  = BREAK,
    LAST_KEYW   = WHILE,
    NUM_KEYW    = (LAST_KEYW - FIRST_KEYW + 1),
    FIRST_SATOM = COMMA,
    LAST_SATOM  = DOT,
    NUM_SATOMS  = LAST_SATOM-FIRST_SATOM} atomType;

char *keyNames[NUM_KEYW]={
        "break"     ,
        "char"      ,
        "double"    ,
        "else"      ,
        "for"       ,
        "if"        ,
        "int"       ,
        "return"    ,
        "struct"    ,
        "void"      ,
        "while"
};

char *atomNames[NUM_ATOMS]={
        "BREAK"     ,
        "CHAR"      ,
        "DOUBLE"    ,
        "ELSE"      ,
        "FOR"       ,
        "IF"        ,
        "INT"       ,
        "RETURN"    ,
        "STRUCT"    ,
        "VOID"      ,
        "WHILE"     ,
        "CT_INT"    ,
        "CT_REAL"   ,
        "ID"        ,
        "ESC"       ,
        "CT_CHAR"   ,
        "CT_STRING" ,
        "COMMA"     ,
        "SEMICOL"   ,
        "LPAR"      ,
        "RPAR"      ,
        "LBRACK"    ,
        "RBRACK"    ,
        "LACC"      ,
        "RACC"      ,
        "ADD"       ,
        "SUB"       ,
        "MUL"       ,
        "DOT"       ,
        "DIV"       ,
        "AND"       ,
        "OR"        ,
        "NOT"       ,
        "ASSIGN"    ,
        "EQUAL"     ,
        "NEQUAL"    ,
        "LESS"      ,
        "LESSEQ"    ,
        "GRT"       ,
        "GRTEQ"     ,
        "END"
};



