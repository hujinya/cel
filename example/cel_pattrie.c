#include "cel/pattrie.h"

void pattrie_print(int level, CelPatTrieNode *node)
{  
    int i;
    CelList *list;
    CelPatTrieNode *child;

    for (i = 0; i < level; i++) 
        printf(" ");
    printf("|");
    for (i = 0; i < level; i++) 
        printf("-");
    printf("%s[%s]#"CEL_CRLF, node->key, (char *)(node->value));

    if ((list = node->static_children) != NULL)
    {
        child = (CelPatTrieNode *)&(list->head);
        while ((child = (CelPatTrieNode *)child->item.next) 
            != (CelPatTrieNode *)cel_list_get_tail(list))
        {
            pattrie_print(level + 1, child);
        }
    }
    if ((list = node->param_children) != NULL)
    {
        child = (CelPatTrieNode *)&(list->head);
        while ((child = (CelPatTrieNode *)child->item.next) 
            != (CelPatTrieNode *)cel_list_get_tail(list))
        {
            pattrie_print(level + 1, child);
        }
    }
}

int pattrie_test(int argc, TCHAR *argv[])
{
    CelPatTrie pat_trie;

    cel_pattrie_init(&pat_trie, NULL);

    cel_pattrie_insert(&pat_trie, "/gopher/bumper.png", "1");
    cel_pattrie_insert(&pat_trie, "/gopher/bumper192x108.png", "2");
    cel_pattrie_insert(&pat_trie, "/gopher/doc.png", "3");
    cel_pattrie_insert(&pat_trie, "/gopher/bumper320x180.png", "4");
    cel_pattrie_insert(&pat_trie, "/gopher/docpage.png", "5");
    cel_pattrie_insert(&pat_trie, "/gopher/doc.png", "6");
    cel_pattrie_insert(&pat_trie, "/gopher/doc", "7");
    cel_pattrie_insert(&pat_trie, "/users/<id>", "8");
    cel_pattrie_insert(&pat_trie, "/users/<id>/profile", "9");
    cel_pattrie_insert(&pat_trie, "/users/<id>/<accnt:\\d+>/address", "10");
    cel_pattrie_insert(&pat_trie, "/users/<id>/age", "11");
    cel_pattrie_insert(&pat_trie, "/users/<id>/<accnt:\\d+>", "12");
    cel_pattrie_insert(&pat_trie, "/users/<id>/test/<name>", "13");
    cel_pattrie_insert(&pat_trie, "/users/abc/<id>/<name>", "14");
    cel_pattrie_insert(&pat_trie, "", "15");
    cel_pattrie_insert(&pat_trie, "/all/<:.*>", "16");

    pattrie_print(0, pat_trie.root);

    cel_pattrie_destroy(&pat_trie);

    return 0;
}
