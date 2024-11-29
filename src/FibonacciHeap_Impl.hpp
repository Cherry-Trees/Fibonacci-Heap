/**
 *  @author Jamie Miles
 *  @date   11/23/2024
 * 
 *  Fibonacci heap Implementation file
 */

#include "FibonacciHeap.hpp"

template<class T, class Key, class Compare>
FibonacciHeap<T, Key, Compare>::Node::Node(): 
    value(), 
    prev(this), 
    next(this), 
    parent(0), 
    child(0), 
    degree(0),
    marked(0) {}

template<class T, class Key, class Compare>
FibonacciHeap<T, Key, Compare>::Node::Node(const T &value): 
    value(value), 
    prev(this), 
    next(this), 
    parent(0), 
    child(0), 
    degree(0),
    marked(0) {}

template<class T, class Key, class Compare>
typename FibonacciHeap<T, Key, Compare>::Node *FibonacciHeap<T, Key, Compare>::Node::join(Node *u) {

    // Return this if the node is null.
    if (!u) return this;

    // Link outside nodes.
    next->prev = u->prev;
    u->prev->next = next;

    // Link this and other nodes.
    next = u;
    u->prev = this;

    // Return the top.
    return Compare{}(Key{}(value), Key{}(u->value)) ? this : u;
}

template<class T, class Key, class Compare>
typename FibonacciHeap<T, Key, Compare>::Node *FibonacciHeap<T, Key, Compare>::Node::cut(Node *u) {
    if (!u) return this;
    if (u->parent) {
        if (u == u->next) {
            u->parent->child = 0;
        } else {
            // Link the parent's child pointer to the next node in the list.
            u->parent->child = u->next;

            // Link outside nodes.
            u->next->prev = u->prev;
            u->prev->next = u->next;
        }
        // Isolate node.
        u->prev = u->next = u;
        --u->parent->degree;
    }
    // Join the node to this root list.
    return u->join(this);
}

template<class T, class Key, class Compare>
FibonacciHeap<T, Key, Compare>::Node *FibonacciHeap<T, Key, Compare>::cut(Node *u) {
    // Unmark the node, cut it, and join it to the root list.
    u->marked = 0;
    _top = _top->cut(u);
    return _top;
}

template<class T, class Key, class Compare>
void FibonacciHeap<T, Key, Compare>::cascade_cut(Node *u) {
    // Skip if the node doesn't have a parent.
    if (u->parent) {
        // If the node is marked, cut it and recursively call with its parent.
        if (u->marked) {
            cut(u);
            cascade_cut(u->parent);
        }
        // Mark the node.
        u->marked = 1;
    }
}

template<class T, class Key, class Compare>
int FibonacciHeap<T, Key, Compare>::Node::add_child(Node *u) {

    // Isolate node.
    u->next->prev = u->prev;
    u->prev->next = u->next;
    u->prev = u->next = u;

    // Parent.
    u->parent = this;

    // Join to the rest of the children.
    child = u->join(child);
    return ++degree;
}

template<class T, class Key, class Compare>
FibonacciHeap<T, Key, Compare>::FibonacciHeap(): _top(0) {}

template<class T, class Key, class Compare>
FibonacciHeap<T, Key, Compare>::~FibonacciHeap() {clear();}

template<class T, class Key, class Compare>
FibonacciHeap<T, Key, Compare>::Node *FibonacciHeap<T, Key, Compare>::push(const T &value) {
    Node *node = new Node(value);
    _top = node->join(_top);
    return node;
}

template<class T, class Key, class Compare>
T &FibonacciHeap<T, Key, Compare>::top() {return _top->value;}

template<class T, class Key, class Compare>
T FibonacciHeap<T, Key, Compare>::top() const {return _top->value;}

template<class T, class Key, class Compare>
T FibonacciHeap<T, Key, Compare>::pop() {

    // Return value.
    T value = _top->value;

    // If the heap has only one thing in it, delete top.
    if (_top == _top->next && !_top->child) {
        delete _top;
        _top = 0;
        return value;
    }

    // Remove top and root all of its children => O(logn)
    _top->join(_top->child); // Root the top node's children.
    _top->child = 0;

    // Temporary top is the next one in line.
    Node *curr = _top->next, *end = _top->next, *temp = _top->next;

    // Link outside nodes.
    _top->next->prev = _top->prev;
    _top->prev->next = _top->next;
    delete _top;
    _top = temp;

    // Group heaps of the same degree => O(logn)
    // Create the degree map.
    const unsigned int max_degree = 32; // Note: Heap max size ≈ 2^32
    Node *degree_map[max_degree];
    for (int i = 0; i < max_degree; degree_map[i] = 0, i++);

    /*  Map one or zero rooted nodes of each degree to the map.
        If more than one node with the same degree is rooted,
            assign one to be the child of the other. */
    bool mapped = 0; // Flag to track if the heaps are mapped.

    // Iterate until the forest's degrees are mapped one-to-one.
    while (!mapped) {
        mapped = 1;
        curr = _top, end = _top; // Start at the top.
        // Look through each node to see if it's in the map.
        do {
            // Place current node in the map if its degree indexes to null.
            if (!degree_map[curr->degree]) {
                degree_map[curr->degree] = curr;
                curr = curr->next;
            }
            // If the current node is the same as the one mapped, skip.
            else if (degree_map[curr->degree] == curr) {
                curr = curr->next;
            }
            // Union the current node with the one found in the map.
            else {
                mapped = 0; // We aren't fully mapped.
                // Find which one's the parent and which one's the child by comparing their values.
                Node *parent, *child;
                if (Compare{}(Key{}(curr->value), Key{}(degree_map[curr->degree]->value))) {
                    parent = curr;
                    child = degree_map[curr->degree];
                }
                else {
                    parent = degree_map[curr->degree];
                    child = curr;
                }
                degree_map[curr->degree] = 0; // Reset the degree slot.
                curr = curr->next; // Move to the next node.
                // If the next node is the child itself, move again.
                if (curr == child) 
                    curr = curr->next;
                // If the end marker node is the child, assign a new end marker and top.
                if (end == child) 
                    end = _top = parent;

                // Add the child to the parent's children.
                parent->add_child(child);
            }
        } while (curr != end); // Check if we've gone a full cycle through the list.
    }

    // Find the new top by comparing values and unparent root nodes.
    curr = end = _top;
    do {
        if (Compare{}(Key{}(curr->value), Key{}(_top->value)))
            _top = curr;

        curr->parent = 0;
        curr->marked = 0;
        curr = curr->next;
    } while (curr != end);
    return value;
}

template<class T, class Key, class Compare>
template<class K>
void FibonacciHeap<T, Key, Compare>::decrease_key(Node *node, const K &key) {
    // Update the node's key.
    Key{}(node->value) = key;

    // If the node's new key is less than it's parent's key, root the node and cascade up.
    if (node->parent && Compare{}(key, Key{}(node->parent->value))) {
        cut(node);
        cascade_cut(node->parent);
    }

    // If the node's new key is less than the top's key, make it the new top.
    if (Compare{}(key, Key{}(_top->value)))
        _top = node;
}

template<class T, class Key, class Compare>
bool FibonacciHeap<T, Key, Compare>::empty() const {return !_top;}

template<class T, class Key, class Compare>
void FibonacciHeap<T, Key, Compare>::Node::clear(Node *u) {

    // Return if the node is null.
    if (!u) return;

    // Cut the circular list.
    u->prev->next = 0;
    Node *prev = 0, *curr = u;
    while (curr) {
        clear(curr->child); // Clear children.
        prev = curr;
        curr = curr->next;
        delete prev;
    }
}

template<class T, class Key, class Compare>
void FibonacciHeap<T, Key, Compare>::clear() {
    Node::clear(_top); // Clear top node.
    _top = 0;
}
