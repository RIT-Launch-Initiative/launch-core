/*******************************************************************************
*
*  Name: queue_node.h
*
*  Purpose: Defines a queue node.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef QUEUE_NODE_H
#define QUEUE_NODE_H

/// @brief queue node
/// @tparam T   the object type stored in the node
template <typename T>
struct Node {
    Node<T>* prev;
    Node<T>* next;
    T data;
};

#endif
