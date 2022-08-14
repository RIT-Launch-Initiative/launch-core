#include <stdlib.h>
#include <stdio.h>

#include "queue/queue.h"
#include "queue/allocated_queue.h"

bool basic() {
    Queue<int> q;

    // push nodes onto queue
    Node<int> node0;
    node0.data = 0;
    q.push(&node0);

    if(q.size() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.size());
        return false;
    }

    Node<int> node1;
    node1.data = 1;
    q.push(&node1);

    if(q.size() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.size());
        return false;
    }

    Node<int> node2;
    node2.data = 2;
    q.push(&node2);

    if(q.size() != 3) {
        printf("bad size on queue, should be 3 but is %lu\n", q.size());
        return false;
    }

    // pop nodes from queue
    const Node<int>* node;

    node = q.peek();
    if(node->data != 0) {
        printf("bad peek node 0\n");
        return false;
    }

    node = q.pop();
    if(node->data != 0) {
        printf("bad pop node 0\n");
        return false;
    }

    if(q.size() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.size());
        return false;
    }

    node = q.peek();
    if(node->data != 1) {
        printf("bad peek node 1\n");
        return false;
    }

    node = q.pop();
    if(node->data != 1) {
        printf("bad pop node 1\n");
        return false;
    }

    if(q.size() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.size());
        return false;
    }

    node = q.peek();
    if(node->data != 2) {
        printf("bad peek node 2\n");
        return false;
    }

    node = q.pop();
    if(node->data != 2) {
        printf("bad pop node 2\n");
        return false;
    }

    if(q.size() != 0) {
        printf("bad size on queue, should be 0 but is %lu\n", q.size());
        return false;
    }

    return true;
}

bool int_sort(int& fst, int& snd) {
    return fst > snd;
}

bool sorted() {
    SortedQueue<int> q{&int_sort};

    // push nodes onto queue
    Node<int> node0;
    node0.data = 3;
    q.push(&node0);

    if(q.size() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.size());
        return false;
    }

    Node<int> node1;
    node1.data = 1;
    q.push(&node1);

    if(q.size() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.size());
        return false;
    }

    Node<int> node2;
    node2.data = 2;
    q.push(&node2);

    if(q.size() != 3) {
        printf("bad size on queue, should be 3 but is %lu\n", q.size());
        return false;
    }

    // pop nodes from queue
    const Node<int>* node;

    node = q.peek();
    if(node->data != 3) {
        printf("bad peek node 0\n");
        return false;
    }

    node = q.pop();
    if(node->data != 3) {
        printf("bad pop node 0\n");
        return false;
    }

    if(q.size() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.size());
        return false;
    }

    node = q.peek();
    if(node->data != 2) {
        printf("bad peek node 1\n");
        return false;
    }

    node = q.pop();
    if(node->data != 2) {
        printf("bad pop node 1\n");
        return false;
    }

    if(q.size() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.size());
        return false;
    }

    node = q.peek();
    if(node->data != 1) {
        printf("bad peek node 2\n");
        return false;
    }

    node = q.pop();
    if(node->data != 1) {
        printf("bad pop node 2\n");
        return false;
    }

    if(q.size() != 0) {
        printf("bad size on queue, should be 0 but is %lu\n", q.size());
        return false;
    }

    return true;
}

bool prealloc_basic() {
    alloc::Queue<int, 3> q;

    // push onto queue
    q.push(0);

    if(q.size() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.size());
        return false;
    }

    q.push(1);

    if(q.size() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.size());
        return false;
    }

    q.push(2);

    if(q.size() != 3) {
        printf("bad size on queue, should be 3 but is %lu\n", q.size());
        return false;
    }

    // pop off of queue
    int temp;

    if(*(q.peek()) != 0) {
        printf("bad peek on node 0\n");
        return false;
    }

    if(!q.pop(&temp)) {
        printf("pop failed on node 0\n");
        return false;
    }

    if(temp != 0) {
        printf("bad value on pop from node 0, got %i should be 0\n", temp);
        return false;
    }

    if(q.size() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.size());
        return false;
    }

    if(*(q.peek()) != 1) {
        printf("bad peek on node 1\n");
        return false;
    }

    if(!q.pop(&temp)) {
        printf("pop failed on node 1\n");
        return false;
    }

    if(temp != 1) {
        printf("bad value on pop from node 1, got %i should be 1\n", temp);
        return false;
    }

    if(q.size() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.size());
        return false;
    }

    if(*(q.peek()) != 2) {
        printf("bad peek on node 2\n");
        return false;
    }

    if(!q.pop(&temp)) {
        printf("pop failed on node 2\n");
        return false;
    }

    if(temp != 2) {
        printf("bad value on pop from node 2, got %i should be 2\n", temp);
        return false;
    }

    if(q.size() != 0) {
        printf("bad size on queue, should be 0 but is %lu\n", q.size());
        return false;
    }

    return true;
}

bool prealloc_sorted() {
    alloc::SortedQueue<int, 3> q{&int_sort};

    // push onto queue
    q.push(3);

    if(q.size() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.size());
        return false;
    }

    q.push(1);

    if(q.size() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.size());
        return false;
    }

    q.push(2);

    if(q.size() != 3) {
        printf("bad size on queue, should be 3 but is %lu\n", q.size());
        return false;
    }

    // pop off of queue
    int temp;

    if(*(q.peek()) != 3) {
        printf("bad peek on node 0\n");
        return false;
    }

    if(!q.pop(&temp)) {
        printf("pop failed on node 0\n");
        return false;
    }

    if(temp != 3) {
        printf("bad value on pop from node 0, got %i should be 3\n", temp);
        return false;
    }

    if(q.size() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.size());
        return false;
    }

    if(*(q.peek()) != 2) {
        printf("bad peek on node 1\n");
        return false;
    }

    if(!q.pop(&temp)) {
        printf("pop failed on node 1\n");
        return false;
    }

    if(temp != 2) {
        printf("bad value on pop from node 1, got %i should be 2\n", temp);
        return false;
    }

    if(q.size() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.size());
        return false;
    }

    if(*(q.peek()) != 1) {
        printf("bad peek on node 2\n");
        return false;
    }

    if(!q.pop(&temp)) {
        printf("pop failed on node 2\n");
        return false;
    }

    if(temp != 1) {
        printf("bad value on pop from node 2, got %i should be 1\n", temp);
        return false;
    }

    if(q.size() != 0) {
        printf("bad size on queue, should be 0 but is %lu\n", q.size());
        return false;
    }

    return true;
}

int main() {
    if(!basic()) {
        printf("failed basic push/pop test\n");
    } else {
        printf("passed basic push/pop test\n");
    }

    if(!sorted()) {
        printf("failed sorted test\n");
    } else {
        printf("passed sorted test\n");
    }

    if(!prealloc_basic()) {
        printf("failed preallocated basic test\n");
    } else {
        printf("passed preallocated basic test\n");
    }

    if(!prealloc_sorted()) {
        printf("failed preallocated sorted test\n");
    } else {
        printf("passed preallocated sorted test\n");
    }
}
