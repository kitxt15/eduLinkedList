#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "colors.h"
typedef struct Node {
    size_t prev;
    size_t next; //TODO ASK DED ABOUT SIZE_T AND -1 TODO also ask about "size" field opacity
    int val;
} Node;

enum errors  {
  LIST_CAPACITY_OVERFLOW_ERR,
  LIST_FREELIST_CORRUPT_ERR,
  LIST_CYCLE_ERR,
  LIST_INTERVENTION_ERR,
};
struct ListNode{
    Node* data;
    size_t size;
    size_t capacity;
    size_t freehead;
    int (*verify_list)(struct ListNode* list);
};

int verify_list(struct ListNode* list);

void list_constructor(struct ListNode* list, size_t nEl);
void list_deconstructor(struct ListNode* list);
void realloc_list_bigger(struct ListNode* list);
void check_and_realloc_bigger(struct ListNode* list);
void check_and_realloc_less(struct ListNode* list);

void push_front(struct ListNode* list, int val);
void push_back(struct ListNode* list, int val);
void pop_front(struct ListNode* list, int val);
void pop_back(struct ListNode* list, int val);

void insert_before(struct ListNode* list, int val, size_t pos);
void remove_node(struct ListNode* list, size_t pos);

void PrintList(struct ListNode* list);
void FPrintList(struct ListNode* list, FILE* fp);

int dump_init(const char* filename);
void list_dump(struct ListNode* list, const char* filename, const char* command_log);
int dump_finish(const char* filename);


int verify_list(struct ListNode* list){
    int err = 0;
    if(list->capacity > list->size){
        err = LIST_CAPACITY_OVERFLOW_ERR;
    }
    for (size_t i = list->freehead; (int)i!=-1; i=list->data[i].next){
        if(list->data[i].val != '0xNE'){
            err = LIST_FREELIST_CORRUPT_ERR;
        }
        if(list->data[i].next == -1){
            break;
        }
    }

    for (int i = list->data[0].next;  (i!=0); i = list->data[i].next ){
        if (((list->data[i].prev == i) || (list->data[i].next == i)) && (i!=0)) {
            err = LIST_CYCLE_ERR;
        }
    }

    for (int i = list->data[0].next;  (i!=0); i = list->data[i].next ){
        if (list->data[i].val == '0xNE') {
            err = LIST_INTERVENTION_ERR;
        }
    }
    if (err != 0){
        switch (err){
            case LIST_CAPACITY_OVERFLOW_ERR:
                printf(RED "ERROR: LIST_CAPACITY_OVERFLOW_ERR\n" RESET);
                break;
            case LIST_CYCLE_ERR:
                printf(RED "ERROR: LIST_CYCLE_ERR\n" RESET);
                break;
            case LIST_FREELIST_CORRUPT_ERR:
                printf(RED "ERROR: LIST_FREELIST_CORRUPT_ERR\n" RESET);
                break;
            case LIST_INTERVENTION_ERR:
                printf(RED "ERROR: LIST_INTERVENTION_ERR\n" RESET);
                break;
        }
        getchar();
        return err;
    }
    return 0;
}

void list_constructor(struct ListNode* list, size_t nEl){

    void* ptr = calloc(sizeof(Node), nEl);

    *list = (struct ListNode){
        .data     = (Node*)ptr,
        .size     = nEl,
        .capacity = 1,
        .verify_list = verify_list,
    };
    Node fiction_Node = {};
    fiction_Node.next = 0;
    fiction_Node.prev = 0;
    fiction_Node.val  = 0;
    list->data[0]     = fiction_Node;


    list->freehead = 1;
    int i = 1;
    for (; i < nEl; i++){
        Node node     = {};
        node.next     = i+1 != nEl ? i+1 : -1;
        node.prev     = -1;
        node.val      = '0xNE';
        list->data[i] = node;
    }

    list->verify_list(list);

}

void list_deconstructor(struct ListNode *list){
    list->verify_list(list);
    free(list->data);
}


void check_and_realloc_bigger(struct ListNode* list){
    list->verify_list(list);
    if (list->capacity == list->size-1){
        list->data = (Node *)realloc(list->data, list->size*2*sizeof(Node));
        int i = list->capacity;
        for (; i < list->size*2; i++){
            Node node     = {};
            node.next     = i+1 != list->size*2 ? i+1 : -1;
            node.prev     = -1;
            node.val      = '0xNE';
            list->data[i] = node;
        }
        list->size *= 2;
    }
    list->verify_list(list);
}

void check_and_realloc_less(struct ListNode* list){
    list->verify_list(list);
    if (list->capacity <= (list->size)/4){
        for (int i = list->size/2; i < list->size; i++){
            if (list->data[i].val != '0xNE'){
                list->verify_list(list);
                return;
            }
        }
        list->data = (Node *)realloc(list->data, (list->size/2)*sizeof(Node));
        list->size /= 2;
        int i = list->freehead;
        if (i < list->size){
        while(list->data[i].next < list->size){
            i = list->data[i].next;
        }
        list->data[i].next = -1;
        }

    }
    list->verify_list(list);
}


void push_front(struct ListNode* list, int val) {
    list->verify_list(list);
    insert_before(list, val, 0);
    list->verify_list(list);
}

void push_back(struct ListNode* list, int val){
    list->verify_list(list);
    insert_before(list, val, list->data[0].next);
    list->verify_list(list);
}

void pop_front(struct ListNode* list, int val){
    list->verify_list(list);
    remove_node(list, list->data[0].prev);
    list->verify_list(list);
}

void pop_back(struct ListNode *list, int val){
    list->verify_list(list);
    remove_node(list, list->data[0].next);
    list->verify_list(list);
}


void insert_before(struct ListNode *list, int value, size_t pos){
    check_and_realloc_bigger(list);
    list->verify_list(list);
    if (list->data[pos].val == '0xNE'){
        printf("false\n");
        return; //TODO better veirification
    }

    Node node = {};
    node.val = value;

    int new_pos = list->freehead;

    node.next = pos;
    node.prev = list->data[pos].prev;

    list->data[list->data[pos].prev].next = new_pos;
    list->data[pos].prev = new_pos;

    list->freehead = list->data[list->freehead].next;
    list->data[new_pos] = node;

    list->capacity++;

    list->verify_list(list);
}

void remove_node(struct ListNode* list, size_t pos){
    list->verify_list(list);

    list->data[list->data[pos].prev].next =  list->data[pos].next;
    list->data[list->data[pos].next].prev =  list->data[pos].prev;

    list->data[pos].next=list->freehead;
    list->data[pos].prev = -1;
    list->data[pos].val = '0xNE';
    list->freehead = pos;
    list->capacity--;

    list->verify_list(list);
    check_and_realloc_less(list);
}

void PrintList(struct ListNode *list){
    printf("[INDX]: ");
    for (int i = 0; i< list->size; i++){
        printf("[%03d] ", i);
    }

    printf("\n");

    printf("[VALU]: ");
    for (int i = 0; i< list->size; i++){
        if (list->data[i].val == '0xNE'){
            printf(RED "[NUL] " RESET);
        }
        else{
            printf("[%03d] ", list->data[i].val);
        }
    }

    printf("\n");

    printf("[PREV]: ");
    for (int i = 0; i< list->size; i++){
        if (list->data[i].prev == -1){
            printf(RED "[NUL] " RESET);
        }
        else{
            printf("[%03zu] ", list->data[i].prev);
        }
    }

    printf("\n");

    printf("[NEXT]: ");
    for (int i = 0; i< list->size; i++){
        if (list->data[i].next == -1){
            printf(RED "[NUL] " RESET);
        }
        else{
            printf("[%03zu] ", list->data[i].next);
        }
    }
    printf("\n");
}

void FPrintList(struct ListNode* list, FILE* fp){
    fprintf(fp, "<pre>\n");
    fprintf(fp, "<b>[INDX]: </b>");
    for (int i = 0; i< list->size; i++){
        fprintf(fp, "[%03d] ", i);
    }

    fprintf(fp, "\n");

    fprintf(fp, "<b>[VALU]: </b>");
    for (int i = 0; i< list->size; i++){
        if (list->data[i].val == '0xNE'){
            fprintf(fp,  "<span class=\"null-value\">[NUL]</span> " );
        }
        else{
            fprintf(fp, "[%03d] ", list->data[i].val);
        }
    }

    fprintf(fp, "\n");

    fprintf(fp, "<b>[PREV]: </b>");
    for (int i = 0; i< list->size; i++){
        if (list->data[i].prev == -1){
            fprintf(fp,  "<span class=\"null-value\">[NUL]</span> " );
        }
        else{
            fprintf(fp,"[%03zu] ", list->data[i].prev);
        }
    }

    fprintf(fp, "\n");

    fprintf(fp, "<b>[NEXT]: </b>");
    for (int i = 0; i< list->size; i++){
        if (list->data[i].next == -1){
            fprintf(fp,  "<span class=\"null-value\">[NUL]</span> " );
        }
        else{
            fprintf(fp, "[%03zu] ", list->data[i].next);
        }
    }
    fprintf(fp, "\n");
    fprintf(fp, "</pre>\n");
}

int dump_init(const char* filename){
    FILE* fp = fopen(filename, "w");

    if (fp == NULL){
        return -errno;
    }
    const char* init_text = "<!DOCTYPE html>\n<html lang=\"ru\">\n<meta charset=\"UTF-8\">\n<title>Linked List Dumps</title>\n<link rel=\"stylesheet\" href=\"dump.css\">\n<body>\n<h1>Linked List Debugger</h1>\n";
    fprintf(fp, "%s", init_text);
    fprintf(fp, "<link rel=\"stylesheet\" href=\"dump.css\">");
    fclose(fp);
    return 1;
}
void list_dump(struct ListNode* list, const char* filename, const char* command_log){
    static size_t dump_count = 1;
    char gz_static1[1024] = {};
        snprintf(gz_static1, 1024 ,"digraph LinkedList {\n\
        graph [\n\
            rankdir=LR,\n\
    bgcolor=\"transparent\",\n\
    nodesep=0.6,\n\
            ranksep=0.9,\n\
            splines=polyline,\n\
            pad=0.3,\n\
            fontname=\"Helvetica\"\n\
        ];\n\
    \n\
        node [\n\
            shape=Mrecord,\n\
            style=filled,\n\
            fontname=\"Helvetica\",\n\
            fontsize=12,\n\
            penwidth=1.5\n\
        ];\n\
        \n\
        edge [\n\
            arrowsize=0.7,\n\
            penwidth=1.7\n\
        ];\n\
        \n\
        subgraph cluster_used {\n\
            label=\"USED LIST | size = %zu\";\n\
            style=\"rounded,filled\";\n\
            color=\"#99F6E4\";\n\
            fillcolor=\"#F0FDFA\";\n\
            fontcolor=\"#0F766E\";\n\
            fontsize=17;\n\
            margin=25;\n", list->capacity);

    FILE* fp = fopen(filename ,"a");

    char dot_filename[32] = {};
    snprintf(dot_filename, 20, "dump%zu.dot", dump_count);
    FILE* gz_fp = fopen(dot_filename ,"w");

    fprintf(fp, "<h2>Dump %zu</h2>", dump_count);
    FPrintList(list, fp);

    fprintf(gz_fp, "%s", gz_static1);
    fprintf(gz_fp, "n0 [\n\
            label=\"{DUMMY #0|value: 0|{prev: %zu|next: %zu}}\",\n\
            fillcolor=\"#DBEAFE\",\n\
            color=\"#3B82F6\"\n\
        ];\n", list->data[0].prev, list->data[0].next);
    //n2 [label="{NODE #2|value: 17|{prev: 0|next: 1}}"];
    for (size_t i = list->data[0].next, c = 1; i != 0; i = list->data[i].next, c++){
        fprintf(gz_fp, "n%zu[label=\"{NODE #%zu|value: %d|{prev: %zu|next: %zu}}\"];\n", i, i, list->data[i].val, list->data[i].prev, list->data[i].next);
    }
//
// n0 -> n2 [color="#0D9488"];
//    n2 -> n1 [color="#0D9488"];
 //   n1 -> n0 [color="#0D9488", constraint=false];
//
    fprintf(gz_fp, "n0 -> n%zu [color=\"#0D9488\"];\n", list->data[0].next);
    for (size_t i = list->data[0].next, c = 1; i != 0; i = list->data[i].next, c++){
        if (list->data[i].next == 0){
             fprintf(gz_fp, "n%zu -> n%zu [color=\"#0D9488\", constraint=false];\n", i, list->data[i].next);
             continue;
        }
        fprintf(gz_fp, "n%zu -> n%zu [color=\"#0D9488\"];\n", i, list->data[i].next);
    }

    fprintf(gz_fp, "n0:ne -> n%zu:nw [\n\
        color=\"#F59E0B\",\n\
        style=dashed,\n\
        constraint=false\n\
    ];\n", list->data[0].prev);

    // fprintf(gz_fp, "n0 -> n%zu [color=\"#0D9488\"];\n", list->data[0].next);
    for (size_t i = list->data[0].prev; i != 0; i = list->data[i].prev){

        fprintf(gz_fp, "n%zu -> n%zu [\n\
            color=\"#F59E0B\",\n\
            style=dashed,\n\
            constraint=false\n\
        ];\n", i, list->data[i].prev);
    }

    fprintf(gz_fp, "}\n");

    char gz_static2[1024] = {};
    snprintf(gz_static2,1024,
        "subgraph cluster_free {\n\
        label=\"FREE LIST | free_head = %zu\";\n\
        style=\"rounded,filled\";\n\
        color=\"#CBD5E1\";\n\
        fillcolor=\"#F8FAFC\";\n\
        fontcolor=\"#64748B\";\n\
        fontsize=17;\n\
        margin=25;\n\
\n\
        node [\n\
            fillcolor=\"#E2E8F0\",\n\
            color=\"#94A3B8\"\n\
        ];\n", list->freehead);

     fprintf(gz_fp, "%s", gz_static2);

     //n3 [label="{FREE #3|prev: -1|next: 4}"];
     //
     for (size_t i = list->freehead, c = 1; (int)i != -1; i = list->data[i].next, c++){
         fprintf(gz_fp, "n%zu [label=\"{FREE #%zu|prev: -1|next: %d}\"];\n", i, i, (int)list->data[i].next);
     }

     fprintf(gz_fp, "null [\n\
         label=\"NULL\",\n\
         shape=oval,\n\
         fillcolor=\"#FEE2E2\",\n\
         color=\"#EF4444\",\n\
         fontcolor=\"#B91C1C\"\n\
     ];\n");

     fprintf(gz_fp, "edge [color=\"#94A3B8\"];\n");

     // n3 -> n4;
     // n4 -> n5;
     // n5 -> n6;
     // n6 -> n7;
     // n7 -> n8;
     // n8 -> n9;
     // n9 -> null [style=dashed];
     //

     for (size_t i = list->freehead; (int)i != -1; i = list->data[i].next){

         if ((int)list->data[i].next == -1){
             fprintf(gz_fp, "n%zu -> null [style=dashed];\n", i);
             continue;
         }
         fprintf(gz_fp, "n%zu -> n%zu;\n", i, list->data[i].next);
     }




    // fprintf(gz_fp ,"{ rank=same; n0; n%zu; }", list->freehead);
    fprintf(gz_fp, "}\n");
    fprintf(gz_fp, "}\n");
    fclose(gz_fp);

    char command[128]= {};

    snprintf(command, sizeof(command),
             "dot -Tsvg dump%zu.dot -o dump_%zu.svg",
             dump_count, dump_count);

    int result = system(command);

    if (result != 0) {
        fprintf(stderr, "Graphviz failed: %d\n", result);
        return;
    }


    fprintf(fp, "\
    <pre>\n\
    size = %zu\n\
    capacity = %zu\n\
    <span class=\"command-log\">%s</span>\
    </pre>\n\
    <div class=\"graph-container\">\n\
    <img src=\"dump_%zu.svg\">\n\
    </div class=\"graph-container\">\n\
    <hr>\n", list->size, list->capacity, command_log, dump_count);
    fclose(fp);

     dump_count++;

}
int dump_finish(const char* filename){
    FILE* fp = fopen(filename, "a");
    if (fp == NULL){
        return -errno;
    }
    fprintf(fp, "</body>\n</html>");
    fclose(fp);
    return 1;
}
