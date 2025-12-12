/****************************************************/
/* File: main.c                                     */
/* Main program for TINY compiler                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"

/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE FALSE

#include "util.h"
#include "scan.h" 
#include "parse.h" 
#include "analyze.h" 
#include "cgen.h" 

 /* allocate global variables */
int lineno = 0;
FILE* source;
FILE* listing;
FILE* code;

/* allocate and set tracing flags */
int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = FALSE;
int TraceAnalyze = FALSE;
int TraceCode = FALSE;

int Error = FALSE;

// Flex ���� ���� extern ����
extern FILE* yyin;
extern int yylex(void); // yylex�� ������ ������Ÿ�� ���� �߰�

// Flex�� ȣ���ϴ� yywrap �Լ� ���� (Undefined Reference ���� �ذ�)
int yywrap(void) {
    return 1;
}

// Undefined reference to 'parse' ���� �ذ��� ���� parse �Լ� ���� ����
TreeNode* parse(void)
{
    extern TreeNode* savedTree;

    // *** �Է� ��Ʈ�� ���� �ذ�: parse ���� ���� yyin�� source ���� �����ͷ� ���� ���� ***
    if (yyin != source) yyin = source;

    yyparse();
    return savedTree;
}


int main(int argc, char* argv[])
{
    TreeNode* syntaxTree;
    char pgm[120]; /* source code file name */

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    strcpy(pgm, argv[1]);
    if (strchr(pgm, '.') == NULL)
        strcat(pgm, ".tny");

    source = fopen(pgm, "r");
    if (source == NULL)
    {
        fprintf(stderr, "File %s not found\n", pgm);
        exit(1);
    }
    /* normalize input: strip UTF-8 BOM and carriage returns (\r) without tmpfile */
    {
        long pos = ftell(source);
        unsigned char bom[3];
        size_t n = fread(bom, 1, 3, source);
        int hasBom = (n == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF);
        if (!hasBom) {
            /* rewind to start if no BOM */
            fseek(source, pos, SEEK_SET);
        }
        /* create a CR-stripped copy on disk (current directory) */
        FILE* tmp = fopen("tmp_input.tmp", "wb+");
        if (tmp == NULL) {
            fprintf(stderr, "Failed to create temp file\n");
            exit(1);
        }
        int ch;
        while ((ch = fgetc(source)) != EOF) {
            if (ch == '\r') continue; /* drop CR */
            fputc(ch, tmp);
        }
        fclose(source);
        source = tmp;
        rewind(source);
    }

    listing = stdout; /* send listing to screen */
    fprintf(listing, "\nTINY COMPILATION: %s\n", pgm);

#if NO_PARSE
    // ��ĳ�� ���� ��忡���� yyin�� �����ؾ� �մϴ�.
    if (yyin != source) yyin = source;
    while (getToken() != ENDFILE);
#else
    // parse() �Լ� ���ο��� yyin�� �����ǵ��� ������ �����߽��ϴ�.
    syntaxTree = parse();
    if (TraceParse) {
        fprintf(listing, "\nSyntax tree:\n");
        printTree(syntaxTree);
    }
#if !NO_ANALYZE
    if (!Error)
    {
        if (TraceAnalyze) fprintf(listing, "\nBuilding Symbol Table...\n");
        buildSymtab(syntaxTree);
        if (TraceAnalyze) fprintf(listing, "\nChecking Types...\n");
        typeCheck(syntaxTree);
        if (TraceAnalyze) fprintf(listing, "\nType Checking Finished\n");
    }
#if !NO_CODE
    if (!Error)
    {
        char* codefile;
        int fnlen = strcspn(pgm, ".");
        codefile = (char*)calloc(fnlen + 4, sizeof(char));
        strncpy(codefile, pgm, fnlen);
        strcat(codefile, ".py"); // Python �ڵ带 ���� .py Ȯ���ڷ� ����

        code = fopen(codefile, "w");
        if (code == NULL)
        {
            printf("Unable to open %s\n", codefile);
            exit(1);
        }

        codeGen(syntaxTree, codefile);
        fclose(code);
        printf("Python code generated successfully: %s\n", codefile); // ���� �޽��� �߰�
    }
#endif
#endif
#endif

    fclose(source);
    return 0;
}