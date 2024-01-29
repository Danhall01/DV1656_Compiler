#include "Node.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define START_CAPACITY 16
#define BUF_SIZE 256

Node_s* initNodeTree(const char* type, const char* value, int32_t lineno)
{
    Node_s* node = NULL;
#ifdef PRINT_PARSER_TREE
    node = (Node_s*) malloc(sizeof(Node_s));
    node->lineno = lineno;
    node->type   = type;
    node->value  = value;

    node->children = (Node_s**) malloc(sizeof(Node_s* [START_CAPACITY]));
    if (node->children == NULL)
    {
        fprintf(stderr, "[-] Failed to create tree\n");
        exit(1);
    }
    node->capacity = START_CAPACITY;
    node->size     = 0;
#endif
    return node;
}

void addSubTree(Node_s node[static 1], Node_s newNode[static 1])
{   
#ifdef PRINT_PARSER_TREE
    if ((node == NULL) || (newNode == NULL)) return;
    if (node->size == node->capacity)
    {
        void* temp = realloc(node->children, node->capacity * 2);
        if (temp == NULL)
        {
            fprintf(stderr, "[-] Out of memory.\n");
            exit(1);
        }
        node->children = temp;
        node->capacity *= 2;
    }
    node->children[node->size++] = newNode;
#endif
}


void printTree(Node_s node[static 1], int depth)
{
    for (int32_t i = 0; i < depth; ++i)
        printf("  ");
    printf("%s:%s\n", node->type, node->value);
    for (uint32_t i = 0; i < node->size; ++i)
        printTree(node->children[i], depth + 1);
}

static void generateTreeContent(Node_s node[static 1], int count[static 1], FILE file[static 1])
{
    node->id = ++(*count);

    /*char   writebuf[BUF_SIZE] = { '\0' };
    size_t writtenSize;
    if ((writtenSize = snprintf(writebuf,
                                sizeof(writebuf) / sizeof(*writebuf),
                                "n%d [label=\"%s:%s\"];\n",
                                node->id,
                                node->type,
                                node->value))
        < 0)
    {
        fprintf(stderr, "[-] Unable to generate tree (sprintf).");
        exit(1);
    }
    fwrite(writebuf, sizeof(*writebuf), writtenSize, file);

    for (uint32_t i = 0; i < node->size; ++i)
    {
        generateTreeContent(node->children[i], count, file);

        memset(writebuf, 0, BUF_SIZE);
        if ((writtenSize = snprintf(writebuf,
                                    sizeof(writebuf) / sizeof(*writebuf),
                                    "n%d -> n%d\n",
                                    node->id,
                                    node->children[i]->id))
            < 0)
        {
            fprintf(stderr, "[-] Unable to generate tree, recursion\n");
            exit(1);
        }
        fwrite(writebuf, sizeof(*writebuf), writtenSize, file);
    }*/
}
void generateTree(Node_s node[static 1])
{
    FILE* file;
    char* fname = "tree.dot";
    file        = fopen(fname, "w");

    int32_t count      = 0;
    char    writebuf[] = "digraph {\n";
    fwrite(writebuf, sizeof(*writebuf), sizeof(writebuf) - 1, file);

    generateTreeContent(node, &count, file);

    fwrite("}", 1, 1, file);
    fclose(file);

    printf("\nBuilt a parse-tree at %s. Use 'make tree' to generate the pdf version.\n", fname);
}
