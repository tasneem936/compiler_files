#include	"CodeGenerator.h"

int ADDR = 5;
int counter =0;
int Nlevel = 0;
int allow=1;
int flag = 0;
int Declared = 0;
typedef struct variable {

    /* Think! what does a Variable contain? */
    char* name;
    int size;
    int address;
    int nested_level;
    char* type;
    struct variable* next;
} Variable;

typedef struct symbol_table {
    Variable* var;
} Symbol_table;


/*
*	You need to build a data structure for the symbol table
*	I recommend to use linked list.
*
*	You also need to build some functions that add/remove/find element in the symbol table
*/

/* Create a variable node  */
Variable* CreateVar(char* var_name, char* var_type)
{
    Variable* new_var;
    new_var=(Variable*)malloc(sizeof(Variable));
    if(new_var == NULL){
        printf("Memory allocation failed\n");
        exit(1);
    }
    new_var->name=var_name;
    new_var->type=var_type;
    new_var->size = 1;
    new_var->address = ADDR;
    ADDR++;
    new_var->nested_level = Nlevel;
    new_var->next = NULL;
    return new_var;

}

Symbol_table* CreateSymTable (){
    Symbol_table* SymTable=(Symbol_table*)malloc(sizeof(Symbol_table));
    if (!SymTable){
        printf("Memory Allocation failed\n");
        exit(1);
    }
    SymTable->var = NULL;
    return SymTable;
}

/* add variable node to the linked list of variables (symbol table) */
Symbol_table* AddNode(Variable* curr_var, Symbol_table* SymTable)
{
    Variable *temp;
    if((SymTable->var) == NULL){/* when linked list is empty */
        SymTable->var = curr_var;
        return SymTable;
    }
    else{
        temp = SymTable->var;
        while(temp->next != NULL){
            temp = temp->next;/* traverse the list until p is the last node */
        }
        temp->next = curr_var;
    }
    return SymTable;
}

/* -------------------------------------------------------------------------- */
/* go over the linked list (symbol table) and find the wanted variable */
Variable* FindVar(char* var_name, Symbol_table* SymTable)
{
    Variable *temp, *lastInstance;
    lastInstance = NULL;
    temp=SymTable->var;
    while(temp != NULL)
    {
        if (strcmp(temp->name,var_name)==0){ /* search by nested level and name */
            lastInstance = temp;
        }
        temp = temp->next;
    }
    return lastInstance;

}



/* go over the linked list (symbol table) and delete the nested variable */

Symbol_table* RemoveByLevel(Symbol_table* SymT, int level ){
    Variable *temp,*prev,*curr;
    temp = SymT->var;
    while(temp!=NULL && temp->nested_level == level){
        SymT->var = temp->next;
        free(temp);
        temp = SymT->var;
    }
    while(temp!=NULL){
        while(temp!=NULL && temp->nested_level!=level){
            prev = temp;
            temp = temp->next;
        }
        if(!temp){
            return SymT;
        }
        prev->next = temp->next;
        free(temp);
        temp = prev->next;
    }
    return SymT;
}


Symbol_table* SymTable = 0;

/*A funvtion to print the symbol table*/
void printSymT(){
    Variable* temp;
    printf("Name\t type\t size\t address\t Nested level\n");
    temp = SymTable->var;
    while(temp){
        printf("%s\t %s\t %d\t %d\t %d\n",temp->name,temp->type,temp->size,temp->address,temp->nested_level);
        temp = temp->next;
    }

}

/*
*	This recursive function is the main method for Code Generation
*	Input: treenode (AST)
*	Output: prints the Pcode on the console
*/
int  code_recur(treenode *root)
{
    char* vtype;
    int label_A,label_B;
    if_node  *ifn;
    for_node *forn;
    leafnode *leaf;
    Variable *curr_var, *add_var;
    if(allow){
        SymTable = CreateSymTable();
        allow = 0;
    }

    if (!root)
        return SUCCESS;

    switch (root->hdr.which){
        case LEAF_T:
            leaf = (leafnode *) root;
            switch (leaf->hdr.type) {
                case TN_LABEL:
                    /* Maybe you will use it later */
                    break;

                case TN_IDENT:
                    /* variable case */
                    /*
                    *	In order to get the identifier name you have to use:
                    *	leaf->data.sval->str
                    */
                    /*printf("%s\n",leaf->data.sval->str);*/

                    if(strcmp(leaf->data.sval->str,"main")!=0){
                        if(Declared){ /* add only declared variables to the symbol table */
                            curr_var = FindVar((leaf->data.sval->str),SymTable);
                            if((!curr_var) || (curr_var && (curr_var->nested_level != Nlevel))){
                                add_var = CreateVar(leaf->data.sval->str,vtype);
                                SymTable = AddNode(add_var, SymTable);
                            }

                        }
                        else{   /* loading values from symbol table*/
                            curr_var = FindVar((leaf->data.sval->str),SymTable);
                            if(curr_var){
                                printf("ldc %d\n", curr_var->address);
                                if(flag){
                                    printf("ind\n");
                                }
                            }
                            else{
                                add_var = CreateVar(leaf->data.sval->str,vtype);
                                SymTable = AddNode(add_var, SymTable);
                            }

                        }
                    }
                    break;

                case TN_COMMENT:
                    /* Maybe you will use it later */
                    break;

                case TN_ELLIPSIS:
                    /* Maybe you will use it later */
                    break;

                case TN_STRING:
                    /* Maybe you will use it later */
                    break;

                case TN_TYPE:
                    /* Maybe you will use it later */
                    vtype = toksym(root->hdr.tok,0);
                    break;

                case TN_INT:
                    /* Constant case */
                    /*
                    *	In order to get the int value you have to use:
                    *	leaf->data.ival
                    */
                    printf("ldc %d\n", leaf->data.ival);
                    break;

                case TN_REAL:
                    /* Constant case */
                    /*
                    *	In order to get the real value you have to use:
                    *	leaf->data.dval
                    */
                    printf("ldc %f\n", leaf->data.dval);
                    break;
            }
            break;

        case IF_T:
            ifn = (if_node *) root;
            switch (ifn->hdr.type) {

                case TN_IF:
                    /* if case (without else)*/
                    if (ifn->else_n == NULL) {
                        label_A = counter++;
                        code_recur(ifn->cond);
                        if((ifn->cond->hdr.type == TN_IDENT)
                           || (ifn->cond->hdr.type == TN_INDEX)
                           || (ifn->cond->hdr.type == TN_SELECT)){
                            printf("ind\n");
                        }
                        printf("fjp LABEL%d\n",label_A);
                        code_recur(ifn->then_n);
                        printf("LABEL%d:\n",label_A);
                    }
                    else {
                        label_A = counter++;
                        label_B = counter++;
                        /* if - else case*/
                        code_recur(ifn->cond);
                        if((ifn->cond->hdr.type == TN_IDENT)
                           || (ifn->cond->hdr.type == TN_INDEX)
                           || (ifn->cond->hdr.type == TN_SELECT))
                        {
                            printf("ind\n");
                        }
                        printf("fjp LABEL%d\n",label_A);
                        code_recur(ifn->then_n);
                        printf("ujp LABEL%d\n",label_B);
                        printf("LABEL%d:\n",label_A);
                        code_recur(ifn->else_n);
                        printf("LABEL%d:\n",label_B);
                    }
                    break;

                case TN_COND_EXPR:
                    label_A = counter++;
                    label_B = counter++;
                    /* (cond)?(exp):(exp); */
                    code_recur(ifn->cond);
                    if((ifn->cond->hdr.type == TN_IDENT)
                       || (ifn->cond->hdr.type == TN_INDEX)
                       || (ifn->cond->hdr.type == TN_SELECT))
                    {
                        printf("ind\n");
                    }
                    printf("fjp cond_else%d\n",label_A);
                    code_recur(ifn->then_n);
                    if((ifn->then_n->hdr.type == TN_IDENT)
                       || (ifn->then_n->hdr.type == TN_INDEX)
                       || (ifn->then_n->hdr.type == TN_SELECT))
                    {
                        printf("ind\n");
                    }
                    printf("ujp condLabel_end%d\n",label_B);
                    printf("cond_else%d:\n",label_A);
                    code_recur(ifn->else_n);
                    if((ifn->else_n->hdr.type == TN_IDENT)
                       || (ifn->else_n->hdr.type == TN_INDEX)
                       || (ifn->else_n->hdr.type == TN_SELECT))
                    {
                        printf("ind\n");
                    }
                    printf("condLabel_end%d:\n",label_B);
                    break;

                default:
                    /* Maybe you will use it later */
                    code_recur(ifn->cond);
                    code_recur(ifn->then_n);
                    code_recur(ifn->else_n);
            }
            break;

        case FOR_T:
            forn = (for_node *) root;
            switch (forn->hdr.type) {

                case TN_FUNC_DEF:
                    /* Function definition */
                    /* e.g. int main(...) { ... } */
                    /* Look at the output AST structure! */
                    code_recur(forn->init);
                    code_recur(forn->test);
                    code_recur(forn->incr);
                    code_recur(forn->stemnt);
                    break;

                case TN_FOR:
                    /* For case*/
                    /* e.g. for(i=0;i<5;i++) { ... } */
                    /* Look at the output AST structure! */
                    label_A = counter++;
                    label_B = counter++;
                    code_recur(forn->init);
                    printf("for_loop%d:\n",label_A);
                    code_recur(forn->test);
                    printf("fjp for_end%d\n",label_B);
                    code_recur(forn->stemnt);
                    code_recur(forn->incr);
                    printf("ujp for_loop%d\n",label_A);
                    printf("for_end%d:\n",label_B);
                    break;

                default:
                    /* Maybe you will use it later */
                    code_recur(forn->init);
                    code_recur(forn->test);
                    code_recur(forn->stemnt);
                    code_recur(forn->incr);
            }
            break;

        case NODE_T:
            switch (root->hdr.type) {
                case TN_PARBLOCK:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_PARBLOCK_EMPTY:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_TRANS_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_FUNC_DECL:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_FUNC_CALL:
                    /* Function call */
                    if (strcmp(((leafnode*)root->lnode)->data.sval->str, "printf") == 0) {
                        /* printf case */
                        /* The expression that you need to print is located in */
                        /* the currentNode->right->right sub tree */
                        /* Look at the output AST structure! */
                        code_recur(root->rnode->rnode);
                        if(root->rnode->rnode->hdr.which == LEAF_T){
                            if(root->rnode->rnode->hdr.type == TN_IDENT){
                                printf("ind\n");
                            }
                        }
                        printf("print\n");

                    }
                    else {
                        /* other function calls - for HW3 */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    break;

                case TN_BLOCK:
                    /* Maybe you will use it later */
                    Nlevel++;
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    SymTable = RemoveByLevel(SymTable, Nlevel);
                    Nlevel--;
                    break;

                case TN_ARRAY_DECL:
                    /* array declaration - for HW2 */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_EXPR_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_NAME_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_ENUM_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_FIELD_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_PARAM_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_IDENT_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_TYPE_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_COMP_DECL:
                    /* struct component declaration - for HW2 */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_DECL:
                    /* structs declaration - for HW2 */
                    Declared = 1;
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    Declared = 0;
                    break;

                case TN_DECL_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_DECLS:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_STEMNT_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_STEMNT:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_BIT_FIELD:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_PNTR:
                    /* pointer - for HW2! */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_TYPE_NME:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_INIT_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_INIT_BLK:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_OBJ_DEF:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_OBJ_REF:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_CAST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_JUMP:
                    if (root->hdr.tok == RETURN) {
                        /* return jump - for HW2! */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    else if (root->hdr.tok == BREAK) {
                        /* break jump - for HW2! */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    else if (root->hdr.tok == GOTO) {
                        /* GOTO jump - for HW2! */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    break;

                case TN_SWITCH:
                    /* Switch case - for HW2! */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_INDEX:
                    /* call for array - for HW2! */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_DEREF:
                    /* pointer derefrence - for HW2! */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                case TN_SELECT:
                    /* struct case - for HW2! */
                    if (root->hdr.tok == ARROW){
                        /* Struct select case "->" */
                        /* e.g. struct_variable->x; */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    else{
                        /* Struct select case "." */
                        /* e.g. struct_variable.x; */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    break;

                case TN_ASSIGN:
                    if(root->hdr.tok == EQ){
                        /* Regular assignment "=" */
                        /* e.g. x = 5; */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                        if (root->rnode->hdr.which == LEAF_T) {
                            if (root->rnode->hdr.type == TN_IDENT) {

                                printf("ind\n");
                            }
                        }
                        printf("sto\n");
                    }
                    else if (root->hdr.tok == PLUS_EQ){
                        /* Plus equal assignment "+=" */
                        /* e.g. x += 5; */
                        code_recur(root->lnode);
                        flag = 1;
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                        printf("add\n");
                        printf("sto\n");
                        flag = 0;

                    }
                    else if (root->hdr.tok == MINUS_EQ){
                        /* Minus equal assignment "-=" */
                        /* e.g. x -= 5; */
                        code_recur(root->lnode);
                        flag = 1;
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                        printf("sub\n");
                        printf("sto\n");
                        flag = 0;
                    }
                    else if (root->hdr.tok == STAR_EQ){
                        /* Multiply equal assignment "*=" */
                        /* e.g. x *= 5; */
                        code_recur(root->lnode);
                        flag = 1;
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                        printf("mul\n");
                        printf("sto\n");
                        flag = 0;
                    }
                    else if (root->hdr.tok == DIV_EQ){
                        /* Divide equal assignment "/=" */
                        /* e.g. x /= 5; */
                        code_recur(root->lnode);
                        flag = 1;
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                        printf("div\n");
                        printf("sto\n");
                        flag = 0;
                    }
                    break;

                case TN_EXPR:
                    flag = 1;
                    switch (root->hdr.tok) {
                        case CASE:
                            /* you should not get here */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            break;

                        case INCR:
                            /* Increment token "++" */
                            /* post increment */

                            if(root->lnode){
                                code_recur(root->lnode);
                                flag = 0;
                                code_recur(root->lnode);
                                flag = 1;
                                code_recur(root->lnode);
                                printf("inc 1\n");
                                printf("sto\n");
                            }
                            else{
                                flag = 0;
                                code_recur(root->rnode);
                                flag = 1;
                                code_recur(root->rnode);
                                printf("inc 1\n");
                                printf("sto\n");
                                code_recur(root->rnode);

                            }
                            break;

                        case DECR:
                            /* Decrement token "--" */
                            if(root->lnode){
                                code_recur(root->lnode);
                                flag = 0;
                                code_recur(root->lnode);
                                flag = 1;
                                code_recur(root->lnode);
                                printf("dec 1\n");
                                printf("sto\n");
                            }
                            else{
                                flag = 0;
                                code_recur(root->rnode);
                                flag = 1;
                                code_recur(root->rnode);
                                printf("dec 1\n");
                                printf("sto\n");
                                code_recur(root->rnode);

                            }

                            break;

                        case PLUS:
                            /* Plus token "+" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("add\n");
                            break;

                        case MINUS:
                            /* Minus token "-" */
                            if(!root->lnode){
                                code_recur(root->rnode);
                                printf("neg\n");
                            }
                            else{
                                code_recur(root->lnode);
                                code_recur(root->rnode);
                                printf("sub\n");
                            }

                            break;

                        case DIV:
                            /* Divide token "/" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("div\n");
                            break;

                        case STAR:
                            /* multiply token "*" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("mul\n");
                            break;

                        case AND:
                            /* And token "&&" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("and\n");
                            break;

                        case OR:
                            /* Or token "||" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("or\n");
                            break;

                        case NOT:
                            /* Not token "!" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("not\n");
                            break;

                        case GRTR:
                            /* Greater token ">" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("grt\n");
                            break;

                        case LESS:
                            /* Less token "<" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("les\n");
                            break;

                        case EQUAL:
                            /* Equal token "==" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("equ\n");
                            break;

                        case NOT_EQ:
                            /* Not equal token "!=" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("neq\n");
                            break;

                        case LESS_EQ:
                            /* Less or equal token "<=" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("leq\n");
                            break;

                        case GRTR_EQ:
                            /* Greater or equal token ">=" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("geq\n");
                            break;

                        default:
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            break;
                    }
                    flag = 0;
                    break;

                case TN_WHILE:
                    /* While case */
                    label_A = counter++;
                    label_B = counter++;
                    printf("while_loop%d:\n",label_A);
                    code_recur(root->lnode);
                    printf("fjp while_end%d\n",label_B);
                    code_recur(root->rnode);
                    printf("ujp while_loop%d\n",label_A);
                    printf("while_end%d:\n",label_B);
                    break;

                case TN_DOWHILE:
                    /* Do-While case */
                    label_A=counter;
                    label_B=counter;
                    counter++;
                    printf("do_while_label%d:\n",label_A);
                    code_recur(root->rnode);
                    code_recur(root->lnode);
                    if((root->lnode->hdr.type == TN_IDENT) || (root->lnode->hdr.type == TN_INDEX) || (root->lnode->hdr.type == TN_SELECT))
                        printf("ind\n");
                    printf("fjp end_loop%d\n",label_B);
                    printf("ujp do_while_label%d\n",label_A);
                    printf("end_loop%d:\n",label_B);
                    break;

                case TN_LABEL:
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;

                default:
                    code_recur(root->lnode);
                    code_recur(root->rnode);
            }
            break;

        case NONE_T:
            printf("Error: Unknown node type!\n");
            exit(FAILURE);
    }

    return SUCCESS;
}


/*
*	This function prints all the variables on your symbol table with their data
*	Input: treenode (AST)
*	Output: prints the Sumbol Table on the console
*/
void print_symbol_table(treenode *root) {
    code_recur(root);
    printf("\n");
    printf("---------------------------------------\n");
    printf("Showing the Symbol Table:\n");
    /*
    *	add your code here
    */
    printSymT();

}
