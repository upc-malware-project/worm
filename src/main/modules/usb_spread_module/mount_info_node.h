#pragma once

#include "mount_info.h"
#include <stdlib.h>

#include "globals.h"

typedef struct Node {
    Mounts mount;
    struct Node* next;
    struct Node* prev;
} Node;

Node *createNode(Mounts info);

void appendNode(Node **head, Node **tail, Mounts data);

void deleteNode(Node **head, Node **tail, int position);

Mounts* listToArr(Node* head, Node* tail, size_t *count);

// To fix "global" variable, issue with loading crashes
void init_mount_node_glob(Globals * glob);