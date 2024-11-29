#ifndef FIBONACCIHEAP_HPP
#define FIBONACCIHEAP_HPP

/**
 *  @author Jamie Miles
 *  @date   11/23/2024
 * 
 *  Generic Fibonacci heap interface.
 *  Takes three template arguments:
 *  
 *  T: Data type stored in the heap.
 *  Key: Key functor to extract the key from T.
 *  Compare: Comparison functor to compare two keys (how the data is sorted).
 * 
 *  Supports pushing data, popping data, and decreasing keys.
 */

// Default key functor.
template<class T> 
struct DefaultKey {
    static T &operator()(T &value) {
        return value;
    }
};
// Default compare functor.
template<class K> 
struct DefaultCompare {
    static bool operator()(const K &first, const K &second) {
        return first < second;
    }
};
// Fibonacci heap.
template<class T, class Key = DefaultKey<T>, class Compare = DefaultCompare<T>>
class FibonacciHeap {
    public:
        struct Node {
            T value;                // Data.
            Node *prev, *next;      // Next/prev nodes (circular doubly linked list)
            Node *parent, *child;   // Parent/child nodes. Each parent/child is its own circular doubly linked list.
            int degree;             // How many nodes have this node as its parent (How many children it has).
            bool marked;            // Flag for cascade cut operation.

            Node();                 // Default constructor.
            Node(const T &value);   // Constructor that initializes the value.

            Node *join(Node *u);    // Union operation joins the node u to the implicit node's list.
            Node *cut(Node *u);     // Cut operation removes the node u from its list and joins it to the implicit node's list.
            int add_child(Node *u); // Utility method for adding a child to the implicit node.
            static void clear(Node *u); // Clears the data from the node u's list.
        };
    private:
        Node *_top;                 // Top of the heap. Determined by comparing keys.
        Node *cut(Node *u);         // Cut operation removes the node u from its list and joins it to the root list.
        void cascade_cut(Node *u);  // Recursively cuts or marks nodes of a tree starting from a child node.

    public:
        FibonacciHeap();            // Default constructor.
        ~FibonacciHeap();           // Destructor.
        Node *push(const T &value); // Pushes a new node containing the value argument to the root list. Returns a pointer to the new node.
        T &top();                   // Return the top value (rvalue).
        T top() const;              // Return the top value (lvalue).
        T pop();                    // Pop operation removes the top node, restructures the heap, and picks a new top.
        template<class K> void decrease_key(Node *node, const K &key);  // Decrease key operation decreases a node's key.
        bool empty() const;         // Checks if the heap is empty.
        void clear();               // Clears all data from the heap.
};

#endif
