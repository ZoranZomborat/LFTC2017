

typedef struct _Token{
	int code; // codul (numele)
	union{
		char *text; // folosit pentru ID, CT_STRING (alocat dinamic)
		long int i; // folosit pentru CT_INT, CT_CHAR
		double r; // folosit pentru CT_REAL
	};
	int line; // linia din fisierul de intrare
	struct _Token *next; // inlantuire la urmatorul AL
}Token;

enum{ ID, END, CT_INT, ASSIGN, SEMICOLON, BREAK, CHAR, EQUAL }; // codurile AL
