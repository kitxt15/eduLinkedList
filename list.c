#include <stdio.h>

#include "link.h"

int main(){
    printf("Hello world!\n");
    const char* dump_filename = "dump.html";

    struct ListNode list = {};
    list_constructor(&list, 4);

    dump_init(dump_filename);

    list_dump(&list, dump_filename, "init");
    insert_before(&list, 3, 0);
    list_dump(&list, dump_filename, "insert(3,0)");
    insert_before(&list, 4, 1);
    list_dump(&list, dump_filename, "insert(4,1)");
    insert_before(&list, 5, 2);
    list_dump(&list, dump_filename, "insert(5,2)");
    insert_before(&list, 6, 3);
    list_dump(&list, dump_filename, "insert(6,3)");
    remove_node(&list, 4);
    list_dump(&list, dump_filename, "remove(3)");
    remove_node(&list, 3);
    list_dump(&list, dump_filename, "remove(2)");
    remove_node(&list, 2);
    list_dump(&list, dump_filename, "remove(1)");


    dump_finish(dump_filename);

    list_deconstructor(&list);
}
