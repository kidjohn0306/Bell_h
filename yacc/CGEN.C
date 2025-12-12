/****************************************************/
/* File: cgen.c                                     */
/* The code generator implementation                */
/* for the TINY compiler                            */
/* (generates code for the Python language)         */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "cgen.h"
#include <string.h>
#include <stdlib.h>

/* ���� �鿩���� ���� */
static int indentLevel = 0;

/* --- ���� ��ƿ��Ƽ �Լ� --- */

/* �鿩���⸦ ����մϴ�. */
static void printIndent(void)
{
    int i;
    for (i = 0; i < indentLevel; i++)
        fprintf(code, "    "); // Python ǥ�� 4 spaces
}

/* ����: ���� ����� �ڽĵ��� ��������� ��ȸ�ϸ� Python �ڵ带 �����մϴ�. */
/* genExp�� ����Ͽ� ���� ��� ���ڿ��� ��ȯ�մϴ�. */
static char* genExp(TreeNode* tree);

/* genStmt�� ����Ͽ� ���� �ڵ带 ����մϴ�. */
static void genStmt(TreeNode* tree);


/* --- ��(Expression) �ڵ� ���� --- */

/* �� ���(ExpK)�� Python �ڵ带 ���ڿ��� ��ȯ�մϴ�. */
static char* genExp(TreeNode* tree)
{
    if (tree == NULL) return strdup("");

    char* exp_str = NULL;

    switch (tree->kind.exp) {

    case ConstK:
        // Const: ���� ���
        exp_str = (char*)malloc(20); // ����� ũ�� �Ҵ�
        sprintf(exp_str, "%d", tree->attr.val);
        break;

    case IdK:
        // Id: ���� �̸�
        exp_str = strdup(tree->attr.name);
        break;

    case OpK:
        // Op: ���� ������
    {
        char* left = genExp(tree->child[0]);
        char* right = genExp(tree->child[1]);
        const char* op_symbol;

        switch (tree->attr.op) {
        case PLUS: op_symbol = "+"; break;
        case MINUS: op_symbol = "-"; break;
        case TIMES: op_symbol = "*"; break;
        case OVER: op_symbol = "//"; break; // TINY�� ���� �������̹Ƿ� // ���
        case LT: op_symbol = "<"; break;
        case EQ: op_symbol = "=="; break;
        default: op_symbol = "/* UNKNOWN OP */"; break;
        }

        exp_str = (char*)malloc(strlen(left) + strlen(right) + strlen(op_symbol) + 5);
        // ��ȣ �߰� (������ �켱������ ���� �����ϰ�)
        sprintf(exp_str, "(%s %s %s)", left, op_symbol, right);

        free(left);
        free(right);
    }
    break;
    default:
        exp_str = strdup("/* UNKNOWN EXP */");
        break;
    }
    return exp_str;
}


/* --- ����(Statement) �ڵ� ���� --- */

/* ���� ���(StmtK)�� Python �ڵ带 ����մϴ�. */
static void genStmt(TreeNode* tree)
{
    if (tree == NULL) return;

    /* ���� ������ �ڵ带 �����ϰ� ��� */
    printIndent();

    switch (tree->kind.stmt) {

    case IfK:
        // IF exp THEN stmt_sequence [ELSE stmt_sequence] END
    {
        char* test_exp = genExp(tree->child[0]);
        fprintf(code, "if %s:\n", test_exp);
        free(test_exp);

        indentLevel++;
        genStmt(tree->child[1]); // THEN ����
        indentLevel--;

        if (tree->child[2] != NULL) {
            printIndent();
            fprintf(code, "else:\n");
            indentLevel++;
            genStmt(tree->child[2]); // ELSE ����
            indentLevel--;
        }
    }
    break;

    case RepeatK:
        // REPEAT stmt_sequence UNTIL exp
    {
        fprintf(code, "while True:\n"); // Python�� ���� ����

        indentLevel++;
        genStmt(tree->child[0]); // REPEAT ����

        // UNTIL ������ ���̸� break
        char* test_exp = genExp(tree->child[1]);
        printIndent();
        fprintf(code, "if %s: break\n", test_exp);
        free(test_exp);

        indentLevel--;
    }
    break;

    case AssignK:
        // ID := exp
    {
        char* rhs_exp = genExp(tree->child[0]);
        fprintf(code, "%s = %s\n", tree->attr.name, rhs_exp);
        free(rhs_exp);
    }
    break;

    case ReadK:
        // READ ID
        // Python�� input()�� ���ڿ��� ��ȯ�ϹǷ� int()�� ��ȯ
        fprintf(code, "%s = int(sys.stdin.readline())\n", tree->attr.name);
        break;

    case WriteK:
        // WRITE exp
    {
        char* exp_to_write = genExp(tree->child[0]);
        fprintf(code, "print(%s)\n", exp_to_write);
        free(exp_to_write);
    }
    break;

    default:
        fprintf(code, "/* WARNING: Unknown StmtKind */\n");
        break;
    }

    /* sibling (���� ����) ó�� */
    genStmt(tree->sibling);
} /* genStmt */


/* --- �� �ڵ� ���� �Լ� --- */

/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree.
 */
void codeGen(TreeNode* syntaxTree, char* codefile)
{
    // �ڵ� ���� �̸� ��� (�ּ�)
    fprintf(code, "# TINY to Python Code Generated from %s\n", codefile);
    fprintf(code, "# K.C. Louden Compiler Project\n\n");

    // Python �ڵ� ������ ���� ǥ�� ��� ����Ʈ (readk ������ sys.stdin.readline ���)
    fprintf(code, "import sys\n\n");

    // TINY ���� ���� ������ �����Ƿ�, ���� ��� ������ 0���� �ʱ�ȭ
    // �̴� buildSymtab()�� ������ Ȱ���� �� �־�� �մϴ�. (����� �ɺ� ���̺� ������ ����)
    // ������ ���÷� �ּ� ó��:
    // fprintf(code, "# Initializing variables based on symbol table...\n");
    // (���� st_lookup�̳� symbol table�� Ȱ���� �� �ִٸ� ���⿡ �ʱ�ȭ �ڵ尡 ���Ե�)

    /* TINY ���α׷� �ڵ带 ��������� ���� */
    indentLevel = 0;
    genStmt(syntaxTree);

    fprintf(code, "\n# End of TINY program execution\n");
}