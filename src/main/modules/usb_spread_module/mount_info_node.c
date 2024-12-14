#include "mount_info_node.h"
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

Node *createNode(Mounts info) {
    Node *newNode = global->malloc(sizeof(Node));
    if (!newNode) {
        DEBUG_LOG_ERR("[USB] fail to alloc memory for mount createnode\n");
        return NULL;
    }

    newNode->mount = info;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

void appendNode(Node **head, Node **tail, Mounts data) {
    Node *newNode = createNode(data);
    if (*tail == NULL) {
        *head = newNode;
        *tail = newNode;
    } else {
        newNode->prev = *tail;
        newNode->next = NULL; //redundante
        (*tail)->next = newNode;
        *tail = newNode;
    }
}

void deleteNode(Node **head, Node **tail, int position) {
    if (*head == NULL) {
        return;
    }

    Node *current = *head;
    int index = 0;

    // Go to specified position
    while (current != NULL && index < position) {
        current = current->next;
        index++;
    }

    if (current == NULL) {
        return;
    }

    // If the node to be deleted is the head
    if (*head == current) {
        *head = current->next;
        if (*head ==  NULL) {
            //there was only 1 element
            *head = NULL; //redundant
            *tail = NULL;
        } else {
            (*head)->prev = NULL;
        }
    } else if (*tail == current) {
        // node is in the tail
      *tail = current->prev;
      (*tail)->next = NULL;
    } else {
        //node in the middle
        (current->prev)->next = current->next;
        (current->next)->prev = current->prev;
    }

    global->free(current);
}

Mounts* listToArr(Node* head, Node* tail, size_t *countArr) {
    size_t count = 0;
    Node* current = head;
    
    while (current != NULL) {
        count++;
        current = current->next;
    }

    *countArr = count;

    Mounts* arr = (Mounts *) global->malloc(count * sizeof(Mounts));
    if (!arr) {
        DEBUG_LOG_ERR("[USB] fail to alloc mem for mounts list\n");
        return NULL;
    }

    size_t i = 0;
    current = head;
    while (current != NULL) {
        arr[i] = current->mount;
        i++;
        current = current->next;
    }

    return arr;    
}

// To fix "global" variable, issue with loading crashes
void init_mount_node_glob(Globals * glob) {
    global = glob;
}