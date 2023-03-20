#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_TOKENS (10000)
#define MAX_LINE_LENGTH (10000)
#define EXIT_SUCCESS 0

/*
E_parser	→	TE'
E_parser'	→	+TE' | eps
T_parser	→	FT'
T_parser'	→	*FT' | eps
F_parser	→	( E_parser ) | var | n
*/
char tokens[1000][100];
int token_i=0;
char tokens[1000][100];
char keywords[10][10] = {"for", "int", "read", "write"};
char variables[100][100];
int var_val[100];
int variable_count = 0;
bool declaration = false;
bool isFor = false;
FILE *out_file;
struct Node{
    char name[100];
    struct Node* children[20];
    int size,val;
};

struct Node* E_parser();
struct Node* Ed_parser();
struct Node* T_parser();
struct Node* Td_parser();
struct Node* F_parser();
struct Node* X_parser();
struct Node* Xd_parser();
struct Node* C_parser();
struct Node* Cd_parser();
struct Node* A_parser();
struct Node *for_parser();

void assign_name_size(struct Node* n, char s[], int size)
{
    int i;
    for(i = 0; s[i] != '\0'; i++)
        n->name[i] = s[i];
    n->name[i] = '\0';
    n->size = size;
    for(int i = 0; i<size; i++)
        n->children[i] = (struct Node*)malloc(sizeof(struct Node));
}
void error(char e[])
{
    printf("Compilation Error : %s\n",e);
    exit(0);
}

bool verify_variable(char s[])
{
    for(int j = 0;j<variable_count;j++)
    {
        if(strcmp(variables[j],s)==0)
            return true;
    }
    return false;
}

void add_variable(char s[])
{
    if(verify_variable(s))
        error("Variable Declared More Than Once.");
    int j;
    for(j = 0;s[j] != '\0'; j++)
        variables[variable_count][j] = s[j];
    variables[variable_count][j] = '\0';
    variable_count++;
}

struct Node* num_parser()
{
    struct Node* curr = (struct Node*)malloc(sizeof(struct Node));
    assign_name_size(curr,"num",1);
    curr->children[0]->size = 0;
    int j;
    for(j = 0; tokens[token_i][j] != '\0'; j++)
    {
        if(tokens[token_i][j] < '0' || tokens[token_i][j] > '9')
            error("Invalid Number.");
        curr->children[0]->name[j] = tokens[token_i][j];
    }
    curr->children[0]->name[j] = '\0';
    return curr;
}

struct Node* var_parser()
{
    struct Node* curr = (struct Node*)malloc(sizeof(struct Node));
    assign_name_size(curr,"var",1);
    curr->children[0]->size = 0;
    int j;

    for(j = 0; tokens[token_i][j] != '\0'; j++)
    {
        if(tokens[token_i][j] < 'a' || tokens[token_i][j] > 'z')
            error("Invalid Variable or Number.");
        curr->children[0]->name[j] = tokens[token_i][j];
    }
    curr->children[0]->name[j] = '\0';
    for(int j = 0; j<4; j++)
    {
        if(strcmp(curr->children[0]->name, keywords[j])==0)
            error("Variable Name Matching With Keyword.");
    }
    if(declaration)
        add_variable(curr->children[0]->name);
    else
    {
        if(!verify_variable(curr->children[0]->name))
            error("Variable Not Found.");
    }
    return curr;
}

struct Node* read_parser()
{
    if(strcmp(tokens[token_i],"read") != 0)
        error("Syntax Error In Read Statement.");
    struct Node* curr = (struct Node*)malloc(sizeof(struct Node));
    assign_name_size(curr, "R", 3);
    assign_name_size(curr->children[0],"read",0);
    token_i++;
    curr->children[1] = var_parser();
    if(strcmp(tokens[token_i+1],";") != 0)
        error("';' Missing After Read Statement.");
    assign_name_size(curr->children[2],";",0);
    token_i += 2;
    return curr;
}

struct Node* write_parser()
{
    if(strcmp(tokens[token_i],"write") != 0)
        error("Syntax Error In Write Statement.");
    struct Node* curr = (struct Node*)malloc(sizeof(struct Node));
    assign_name_size(curr, "W", 3);
    assign_name_size(curr->children[0],"write",0);
    token_i++;
    if(tokens[token_i][0] >= 'a' && tokens[token_i][0] <='z')
        curr->children[1] = var_parser();
    else
        curr->children[1] = num_parser();
    if(strcmp(tokens[token_i+1],";") != 0)
        error("';' Missing After Write Statement.");
    assign_name_size(curr->children[2],";",0);
    token_i += 2;
    return curr;
}

struct Node* varlist_parser()
{
    struct Node* curr = (struct Node*)malloc(sizeof(struct Node));
    if(tokens[token_i+1][0]==',')
    {
        assign_name_size(curr, "varlist", 3);
        curr->children[0] = var_parser();
        assign_name_size(curr->children[1],",",0);
        token_i += 2;
        curr->children[2] = varlist_parser(token_i);
    }
    else if(tokens[token_i+1][0]==';')
    {
        assign_name_size(curr, "varlist", 1);
        curr->children[0] = var_parser();
    }
    else
        error("Invalid Declaration Statement.");
    return curr;
}

struct Node* dec_parser()
{
    if(strcmp(tokens[token_i],"int") != 0)
        error("Invalid Declaration Statement.");
    struct Node* curr = (struct Node*)malloc(sizeof(struct Node));
    assign_name_size(curr, "D", 3);
    assign_name_size(curr->children[0],"int",0);
    token_i++;
    curr->children[1] = varlist_parser();
    if(strcmp(tokens[token_i+1],";") != 0)
        error("';' Missing After Declaration Statement.");
    assign_name_size(curr->children[2],";",0);
    token_i += 2;
    return curr;
}

struct Node* parser()
{
    struct Node* curr = (struct Node*)malloc(sizeof(struct Node));
    assign_name_size(curr,"S",2);
    if(strcmp(tokens[token_i],"read") == 0)
        curr->children[0]  = read_parser();
    else if(strcmp(tokens[token_i],"write") == 0)
        curr->children[0]  = write_parser();
    else if(verify_variable(tokens[token_i]))
    {
        curr->children[0] = A_parser();
    }
    else if(strcmp(tokens[token_i],"for")==0)
    {
        curr->children[0] = for_parser();
    }
    else{
        printf("%s", tokens[token_i]);
        error("Invalid Statement.");
    }
    if(tokens[token_i][0] == '}')
    {
        curr->size = 1;
        free(curr->children[1]);
        curr->children[1] = NULL;
        return curr;
    }
    if(tokens[token_i][0] != '\0')
        curr->children[1]  = parser();
    else
    {
        curr->size = 1;
        free(curr->children[1]);
        curr->children[1] = NULL;
    }
    return curr;
}

struct Node* program_parser()
{
    struct Node* curr = (struct Node*)malloc(sizeof(struct Node));
    if(strcmp(tokens[token_i],"int") == 0)
    {
        declaration = true;
        assign_name_size(curr,"P",2);
        curr->children[0] = dec_parser();
        declaration = false;
        if(tokens[token_i][0] != '\0')
            curr->children[1] = parser();
        else
        {
            curr->size = 1;
            free(curr->children[1]);
            curr->children[1] = NULL;
        }
    }
    else
        curr = parser();
    return curr;
}

int find_var(char a[])
{
    for(int i=0;i<100;i++)
    {
        if(strcmp(a,variables[i])==0) return i;
    }
    return 0;
}


struct Node* E_parser()
{
    struct Node *node = (struct Node*)malloc(sizeof(struct Node));
    assign_name_size(node,"E",2);
    node->children[0] = T_parser();
    node->children[1] = Ed_parser();
    return node;
}
struct Node* P_parser(){
    struct Node *node = (struct Node*)malloc(sizeof(struct Node));
    assign_name_size(node,"P",1);
    if(tokens[token_i][0]=='+') assign_name_size(node->children[0],"+",0);
    else assign_name_size(node->children[0],"-",0);
    token_i++;
    return node;
}
struct Node* Ed_parser()
{
    struct Node *node = (struct Node*)malloc(sizeof(struct Node));
    if(tokens[token_i][0]!='+' && tokens[token_i][0]!='-')
    {
        assign_name_size(node,"Ed",1);
        assign_name_size(node->children[0],"e1",0);
        return node;
    }
    // token_i++;
    assign_name_size(node,"Ed",3);
    node->children[0] = P_parser();
    node->children[1] = T_parser();
    node->children[2] = Ed_parser();
    return node;
}
struct Node* T_parser()
{
    struct Node *node = (struct Node*)malloc(sizeof(struct Node));
    assign_name_size(node,"T",2);
    node->children[0] = F_parser();
    node->children[1] = Td_parser();
    return node;
}
struct Node* M_parser(){
    struct Node *node = (struct Node*)malloc(sizeof(struct Node));
    assign_name_size(node,"M",1);
    if(tokens[token_i][0]=='*') assign_name_size(node->children[0],"*",0);
    else assign_name_size(node->children[0],"/",0);
    token_i++;
    return node;
}
struct Node* Td_parser()
{
    struct Node *node = (struct Node*)malloc(sizeof(struct Node));
    if(tokens[token_i][0]!='*' && tokens[token_i][0]!='/')
    {
        assign_name_size(node,"Td",1);
        assign_name_size(node->children[0],"e1",0);
        return node;
    }
    // token_i++;
    assign_name_size(node,"Td",3);
    node->children[0] = M_parser();
    node->children[1] = F_parser();
    node->children[2] = Td_parser();
    return node;
}
struct Node* F_parser()
{
    // printf("hello");
    struct Node *node = (struct Node*)malloc(sizeof(struct Node));
    strcpy(node->name,"F");
    if(tokens[token_i][0]=='(')
    {
        token_i++;
        assign_name_size(node,"F",3);
        node->children[1] = C_parser();
        assign_name_size(node->children[0],"(",0);
        assign_name_size(node->children[2],")",0);
        // if(tokens[token_i]!=")") error("Brackets Don't Match.");
        token_i++;
        return node;
    }
    int x = find_var(tokens[token_i]);
    assign_name_size(node,"F",1);
    // assign_name_size(node->children[0],tokens[token_i++],0);
    if(!isdigit(tokens[token_i][0]))
    {
        node->children[0] = var_parser();
    // printf("hello");
    }
    else
    {
        node->children[0] = num_parser();
    }
    token_i++;
    return node;
}

struct Node *Xd_parser()
{
    struct Node *curr = (struct Node *)malloc(sizeof(struct Node));
    if (!(tokens[token_i][0] == '=' && tokens[token_i+1][0] == '=')){
        assign_name_size(curr,"Xd",1);
        assign_name_size(curr->children[0],"e1",0);
        return curr;
    }
    assign_name_size(curr,"Xd",3);

    assign_name_size(curr->children[0],"==",0);
    token_i+=2;

    curr->children[1] = E_parser();

    curr->children[2] =Xd_parser();

    return curr;
}
struct Node *X_parser()
{
    struct Node *curr = (struct Node *)malloc(sizeof(struct Node));
    assign_name_size(curr, "X", 2);

    curr->children[0] = E_parser();
    curr->children[1] = Xd_parser();
    return curr;
}
struct Node *Cd_parser()
{
    struct Node *curr = (struct Node *)malloc(sizeof(struct Node));

    if (!(tokens[token_i][0] == '>')){
        assign_name_size(curr,"Cd",1);
        assign_name_size(curr->children[0],"e1",0);
        // token_i++;
        return curr;
    }
    assign_name_size(curr,"Cd",3);

    if (strcmp(tokens[token_i], ">") != 0)
        error("Invalid Contditional Statemant.");
    assign_name_size(curr->children[0],">",0);
    token_i++;

    curr->children[1] = X_parser();

    curr->children[2] = Cd_parser();

    return curr;
}
struct Node *C_parser()
{
    struct Node *curr = (struct Node *)malloc(sizeof(struct Node));
    assign_name_size(curr, "C", 2);
    curr->children[0] = X_parser();

    curr->children[1] = Cd_parser();

    return curr;
}
struct Node *A_parser()
{
    // printf("here");
    struct Node *curr = (struct Node *)malloc(sizeof(struct Node));
    if (isFor == true)
        assign_name_size(curr, "A2", 3);
    else
        assign_name_size(curr, "A", 4);

    curr->children[0] = var_parser();

    if (strcmp(tokens[++token_i], "=") != 0)
        error("Invalid Assignment Statement.");
    assign_name_size(curr->children[1], "=", 0);
    token_i++;
    curr->children[2] = C_parser();
    if (isFor == false){
        if (strcmp(tokens[token_i], ";") != 0)
            error("';' Missing After Assignment Statement.");
        assign_name_size(curr->children[3], ";", 0);
        token_i++;
    }
    return curr;
}

struct Node *for_parser()
{
    isFor = true;
    if (strcmp(tokens[token_i], "for") != 0)
        error("Invalid for Statement.");
    struct Node *curr = (struct Node *)malloc(sizeof(struct Node));
    assign_name_size(curr, "FOR", 12);
    assign_name_size(curr->children[0], "for", 0);

    if (strcmp(tokens[++token_i], "(") != 0)
        error("Invalid for Statement.");
    assign_name_size(curr->children[1], "(", 0);
    token_i++;

    if (strcmp(tokens[token_i+1],"=")!=0){
        curr->children[2] = C_parser();
    }
    else if (strcmp(tokens[token_i+2],"=")==0){
        curr->children[2] = C_parser();
    }
    else{
        curr->children[2] = A_parser();
    }

    if (strcmp(tokens[token_i], ";") != 0)
        error("Invalid for Statement.");
    assign_name_size(curr->children[3], ";", 0);
    token_i++;

    curr->children[4] = C_parser();

    if (strcmp(tokens[token_i], ";") != 0)
        error("Invalid for Statement.");
    assign_name_size(curr->children[5], ";", 0);
    token_i++;

    if (strcmp(tokens[token_i+1],"=")!=0){
        curr->children[6] = C_parser();
    }
    else if (strcmp(tokens[token_i+2],"=")==0){
        curr->children[6] = C_parser();
    }
    else{
        curr->children[6] = A_parser();
    }

    if (strcmp(tokens[token_i], ")") != 0)
        error("Invalid for Statement.");
    assign_name_size(curr->children[7], ")", 0);
    token_i++;
    isFor = false;

    if (strcmp(tokens[token_i], "{") != 0)
        error("Invalid for Statement.");
    // printf("Hello");
    assign_name_size(curr->children[8], "{", 0);
    token_i++;

    curr->children[9] = parser(); // S_parser

    if (strcmp(tokens[token_i], "}") != 0)
        error("Invalid for Statement.");
    assign_name_size(curr->children[10], "}", 0);
    token_i++;

    if (strcmp(tokens[token_i], ";") != 0)
        error("Invalid for Statement.");
    assign_name_size(curr->children[11], ";", 0);
    token_i++;
    return curr;
}

/*
simulation
*/
void assign_value(char name[],int val)
{
    for(int j = 0;j<variable_count;j++)
    {
        if(strcmp(variables[j],name)==0)
        {
            var_val[j] = val;
            break;
        }
    }
}

int find_value(char name[])
{
    for(int j = 0;j<variable_count;j++)
    {
        if(strcmp(variables[j],name)==0)
            return var_val[j];
    }
    return 0;
}
// int v_g = 2;
struct Node* find_F(struct Node* n);
struct Node* find_Td(struct Node* n);
struct Node* find_T(struct Node* n);
struct Node* find_Ed(struct Node* n);
struct Node* simulate_expression(struct Node* n);
struct Node* find_Xd(struct Node* n);
struct Node* find_X(struct Node* n);
struct Node* find_Cd(struct Node* n);
struct Node* simulate_condition(struct Node* n);
void simulate_read(struct Node* n);
void simulate_write(struct Node* n);
void simulate(struct Node* n);
void simulate_for(struct Node* n);
void simulate_assignment(struct Node* n);
/*Expression*/
struct Node* find_F(struct Node* n)
{
    if(n->size == 3){
        n->val = simulate_condition(n->children[1])->val;
    }
    else if(strcmp(n->children[0]->name,"var") == 0)
        n->val = find_value(n->children[0]->children[0]->name);
    else
        n->val = atoi(n->children[0]->children[0]->name);
    return n;
}
struct Node* find_Td(struct Node* n)
{
    if(n->size==1)
    {
        n->val = 1;
        return n;
    }
    struct Node* n1 = find_F(n->children[1]);
    struct Node* n2 = find_Td(n->children[2]);
    if(n->children[2]->size>1 && n->children[2]->children[0]->children[0]->name[0]=='/') n->val = n1->val / n2->val;
    else n->val = n1->val * n2->val;
    return n;
}
struct Node* find_T(struct Node* n)
{
    struct Node* n1 = find_F(n->children[0]);
    struct Node* n2 = find_Td(n->children[1]);
    if(n2->size==1)
        n->val = n1->val;
    else
    {
        if(n->children[1]->size>1 && n->children[1]->children[0]->children[0]->name[0]=='/') n->val = n1->val / n2->val;
        else n->val = n1->val * n2->val;
    }
    return n;
}
struct Node* find_Ed(struct Node* n)
{
    if(n->size==1)
    {
        n->val = 0;
        return n;
    }
    struct Node* n1 = find_T(n->children[1]);
    struct Node* n2 = find_Ed(n->children[2]);
    if(n->children[0]->children[0]->name[0]=='-') n1->val = -n1->val;
    n->val = n1->val + n2->val;
    return n;
}
struct Node* simulate_expression(struct Node* n)
{
    // return NULL;
    struct Node* n1 = find_T(n->children[0]);
    struct Node* n2 = find_Ed(n->children[1]);
    if(n2->size==1)
        n->val = n1->val;
    else
        n->val = (n1->val + n2->val);
    return n;
}
/*Expression*/

/*Conditional*/
// struct Node* find_E(){
//     struct Node* e_node = (struct Node*)malloc(sizeof(struct Node));
//     e_node->val = v_g++;
//     return e_node;
// }
struct Node* find_Xd(struct Node* Xd_node){
    if (Xd_node->children[2]->size == 1){
        // return NULL;
        struct Node* e_node = simulate_expression(Xd_node->children[1]);
        // struct Node* e_node = find_E();
        Xd_node->val = e_node->val;
    }
    else{
        // return NULL;
        struct Node* e_node = simulate_expression(Xd_node->children[1]);
        // struct Node* e_node = find_E();
        struct Node* xd_node = find_Xd(Xd_node->children[2]);
        Xd_node->val = e_node->val == xd_node->val;
    }
    return Xd_node;
}
struct Node* find_X(struct Node* x_node){
    if (x_node->children[1]->size == 1){
        // return NULL;
        struct Node* e_node = simulate_expression(x_node->children[0]);
        // struct Node* e_node = find_E();
        x_node->val = e_node->val;
    }
    else{
        struct Node* e_node = simulate_expression(x_node->children[0]);
        // struct Node* e_node = find_E();
        // printf("%d",e_node->val);
        struct Node* xd_node = find_Xd(x_node->children[1]);
        // return NULL;
        x_node->val = e_node->val == xd_node->val;
    }
    return x_node;
}
struct Node* find_Cd(struct Node* Cd_node){
    if (Cd_node->children[2]->size == 1){
        struct Node* x_node = find_X(Cd_node->children[1]);
        // return NULL;
        Cd_node->val = x_node->val;
    }
    else{
        // return NULL;
        struct Node* x_node = find_X(Cd_node->children[1]);
        struct Node* cd_node = find_Cd(Cd_node->children[2]);
        Cd_node->val = x_node->val > cd_node->val;
    }
    return Cd_node;
}
struct Node* simulate_condition(struct Node* c_node){
    if (c_node->children[1]->size == 1){
        struct Node* x_node = find_X(c_node->children[0]);
        c_node->val = x_node->val;
    }
    else{
        struct Node* x_node = find_X(c_node->children[0]);
        // c_node->children[1];
        struct Node* cd_node = find_Cd(c_node->children[1]);
        // return NULL;
        c_node->val = x_node->val > cd_node->val;
    }
    return c_node;
}
/*Conditional*/
void simulate_for(struct Node* n){
    // curlycount++;
    // for(simulate_assignment(n->children[2]);simulate_condition(n->children[4]);simulate_assignment(n->children[6])){
    //     simulate(n->children[9]);
    // }
    if (strcmp(n->children[2]->name,"C")==0)
        simulate_condition(n->children[2]);
    else
        simulate_assignment(n->children[2]);
    while(1){
        struct Node* c_node = simulate_condition(n->children[4]);
        if (c_node->val == 0){
            return;
        }
        simulate(n->children[9]);
        if (strcmp(n->children[6]->name,"C")==0)
            simulate_condition(n->children[6]);
        else
            simulate_assignment(n->children[6]);
    }
}
void simulate_assignment(struct Node* n){
    struct Node *c_node = simulate_condition(n->children[2]);
    if(c_node->val < 0)
    {
        printf("Negative Number Error\n");
        exit(0);
    }
    assign_value(n->children[0]->children[0]->name,c_node->val);
}
void simulate_read(struct Node* n)
{
    n = n->children[1];
    n = n->children[0];
    scanf("%d",&n->val);
    if(n->val < 0)
    {
        printf("Negative Number Error\n");
        exit(0);
    }
    assign_value(n->name, n->val);
}

void simulate_write(struct Node* n)
{
    n = n->children[1];
    if(strcmp(n->name,"var")==0)
    {
        int val = find_value(n->children[0]->name);
        printf("%d\n",val);
    }
    else
        printf("%d\n",atoi(n->children[0]->name));
}

void simulate(struct Node* n)
{
    if(strcmp(n->name,"R")==0)
        simulate_read(n);
    else if(strcmp(n->name,"W")==0)
        simulate_write(n);
    else if (n->size > 2 && strcmp(n->children[1]->name,"=")==0)
        simulate_assignment(n);
    else if(strcmp(n->name,"FOR")==0){
        simulate_for(n);
        return;
    }
    for(int j = 0;j<n->size;j++)
        simulate(n->children[j]);
}
/*
Simulation Ends
*/

void preorder(struct Node*node)
{
    if(node==NULL) return;
    fprintf(out_file,"%s",node->name);
    for(int i=0;i<node->size;i++)
    {
        fprintf(out_file,"[");
        preorder(node->children[i]);
        fprintf(out_file,"]");
    }
}
void tokenize(char* filename) {
    FILE* in_file = fopen(filename, "r");
    int token_i = 0;
    char line[MAX_TOKENS];
    char s[100];
    if(!in_file){
        printf("File not found");
        return;
    }
    while (fgets(line, MAX_LINE_LENGTH, in_file))
    {
        int j=0,i=0;
        for(int k=0;k<100;k++) s[k] = '\0';
        while(line[i] <= 32)i++;
        for (i = 0; i < strlen(line) && line[i]!='\n'; ++i)
        {
            if(line[i]<=' ')
            {
                if(s[0]!='\0')
                {
                    for(int k=0;k<100;k++) tokens[token_i][k] = s[k];
                    // printf("%d %s\n",token_i,tokens[token_i]);

                    token_i++;
                }
                for(int k=0;k<100;k++) s[k] = '\0';
                while(i<strlen(line) && line[i]<=' ')
                {
                    i++;
                    continue;
                }
                j=0;
            }
            while((line[i]>=37 && line[i]<=47) || (line[i]>=59 && line[i]<=63)  || (line[i]>=123 && line[i]<=125))
            {
                if(s[0]!='\0')
                {
                    for(int k=0;k<100;k++) tokens[token_i][k] = s[k];
                    // printf("%d %s\n",token_i,tokens[token_i]);
                    token_i++;
                }
                tokens[token_i][0] = line[i];
                // printf("%d %s\n",token_i,tokens[token_i]);
                i++;
                for(int k=0;k<100;k++) s[k] = '\0';
                while(i<strlen(line) && line[i]<=' ')
                {
                    i++;
                    continue;
                }
                j=0;
                token_i++;
            }
            s[j++] = line[i];
        }
        for(int k=0;k<100;k++) if(s[k]=='\n') s[k] = '\0';
        if(s[0]>32)
        {
            strcpy(tokens[token_i],s);
            // printf("%d %d\n",token_i,tokens[token_i][0]);
            token_i++;
        }
    }

}

void print_string(struct Node n)
{
    if(n.size == 0)
    {
        printf("%s\n",n.name);
        return;
    }
    for(int i = 0; i<n.size; i++)
        print_string(*n.children[i]);
}

int main(int argc, char** argv) {

    if (argc < 2) {
        perror("Pass the name of the input file as the first parameter. e.g.: ./simulator input.txt");
        exit(EXIT_FAILURE);
    }
    out_file = fopen("preorder.txt", "w"); // write only 
    memset(tokens,'\0',sizeof(tokens));
    memset(variables,'\0',sizeof(variables));
    tokenize(argv[1]);
    token_i = 0;
    struct Node* node = program_parser();
    // struct Node* n = x();
    fprintf(out_file,"[");
    preorder(node);
    fprintf(out_file,"]");
    simulate(node);
    return EXIT_SUCCESS;
}