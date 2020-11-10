
#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");
#define SAFEALLOCSZ(var,Type,size) if((var=(Type*)malloc(sizeof(Type)*size))==NULL)err("not enough memory");

char *keyNames[NUM_KEYW]=
{
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
