#include <stdlib.h>
#include <stdio.h>

#include "queue/queue.h"
#include "queue/allocated_queue.h"

bool basic() {
    SimpleQueue<int> q;

    // push nodes onto queue
    Node<int> node0;
    node0.data = 0;
    q.push_node(&node0);

    if(q.num_nodes() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.num_nodes());
        return false;
    }

    Node<int> node1;
    node1.data = 1;
    q.push_node(&node1);

    if(q.num_nodes() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.num_nodes());
        return false;
    }

    Node<int> node2;
    node2.data = 2;
    q.push_node(&node2);

    if(q.num_nodes() != 3) {
        printf("bad size on queue, should be 3 but is %lu\n", q.num_nodes());
        return false;
    }

    // pop nodes from queue
    const Node<int>* node;

    node = q.peek_node();
    if(node->data != 0) {
        printf("bad peek node 0\n");
        return false;
    }

    node = q.pop_node();
    if(node->data != 0) {
        printf("bad pop node 0\n");
        return false;
    }

    if(q.num_nodes() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.num_nodes());
        return false;
    }

    node = q.peek_node();
    if(node->data != 1) {
        printf("bad peek node 1\n");
        return false;
    }

    node = q.pop_node();
    if(node->data != 1) {
        printf("bad pop node 1\n");
        return false;
    }

    if(q.num_nodes() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.num_nodes());
        return false;
    }

    node = q.peek_node();
    if(node->data != 2) {
        printf("bad peek node 2\n");
        return false;
    }

    node = q.pop_node();
    if(node->data != 2) {
        printf("bad pop node 2\n");
        return false;
    }

    if(q.num_nodes() != 0) {
        printf("bad size on queue, should be 0 but is %lu\n", q.num_nodes());
        return false;
    }

    return true;
}

bool int_sort(int& fst, int& snd) {
    return fst > snd;
}

bool sorted() {
    SimpleSortedQueue<int> q{&int_sort};

    // push nodes onto queue
    Node<int> node0;
    node0.data = 3;
    q.push_node(&node0);

    if(q.num_nodes() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.num_nodes());
        return false;
    }

    Node<int> node1;
    node1.data = 1;
    q.push_node(&node1);

    if(q.num_nodes() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.num_nodes());
        return false;
    }

    Node<int> node2;
    node2.data = 2;
    q.push_node(&node2);

    if(q.num_nodes() != 3) {
        printf("bad size on queue, should be 3 but is %lu\n", q.num_nodes());
        return false;
    }

    // pop nodes from queue
    const Node<int>* node;

    node = q.peek_node();
    if(node->data != 3) {
        printf("bad peek node 0\n");
        return false;
    }

    node = q.pop_node();
    if(node->data != 3) {
        printf("bad pop node 0\n");
        return false;
    }

    if(q.num_nodes() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.num_nodes());
        return false;
    }

    node = q.peek_node();
    if(node->data != 2) {
        printf("bad peek node 1\n");
        return false;
    }

    node = q.pop_node();
    if(node->data != 2) {
        printf("bad pop node 1\n");
        return false;
    }

    if(q.num_nodes() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.num_nodes());
        return false;
    }

    node = q.peek_node();
    if(node->data != 1) {
        printf("bad peek node 2\n");
        return false;
    }

    node = q.pop_node();
    if(node->data != 1) {
        printf("bad pop node 2\n");
        return false;
    }

    if(q.num_nodes() != 0) {
        printf("bad size on queue, should be 0 but is %lu\n", q.num_nodes());
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
    if(*(q.peek()) != 0) {
        printf("bad peek on node 0\n");
        return false;
    }

    q.pop();

    if(q.size() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.size());
        return false;
    }

    if(*(q.peek()) != 1) {
        printf("bad peek on node 1\n");
        return false;
    }

    q.pop();

    if(q.size() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.size());
        return false;
    }

    if(*(q.peek()) != 2) {
        printf("bad peek on node 2\n");
        return false;
    }

    q.pop();

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

    q.pop();

    if(q.size() != 2) {
        printf("bad size on queue, should be 2 but is %lu\n", q.size());
        return false;
    }

    if(*(q.peek()) != 2) {
        printf("bad peek on node 1\n");
        return false;
    }

    q.pop();

    if(q.size() != 1) {
        printf("bad size on queue, should be 1 but is %lu\n", q.size());
        return false;
    }

    if(*(q.peek()) != 1) {
        printf("bad peek on node 2\n");
        return false;
    }

    q.pop();

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
