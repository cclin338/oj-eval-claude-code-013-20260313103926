/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
    class Key,
    class T,
    class Compare = std::less<Key>
> class map {
public:
    typedef pair<const Key, T> value_type;

private:
    enum Color { RED, BLACK };

    struct Node {
        value_type *data;
        Node *left;
        Node *right;
        Node *parent;
        Color color;

        Node(const value_type &val, Node *p = nullptr)
            : data(new value_type(val)), left(nullptr), right(nullptr), parent(p), color(RED) {}

        ~Node() {
            delete data;
        }
    };

    Node *root;
    Node *leftmost_node;  // pointer to the smallest element
    Node *rightmost_node; // pointer to the largest element
    size_t node_count;
    Compare comp;

    // Helper: find node by key
    Node* findNode(const Key &key) const {
        Node *curr = root;
        while (curr) {
            if (comp(key, curr->data->first)) {
                curr = curr->left;
            } else if (comp(curr->data->first, key)) {
                curr = curr->right;
            } else {
                return curr;
            }
        }
        return nullptr;
    }

    // Helper: find leftmost node in subtree
    Node* findLeftmost(Node *node) const {
        if (!node) return nullptr;
        while (node->left) {
            node = node->left;
        }
        return node;
    }

    // Helper: find rightmost node in subtree
    Node* findRightmost(Node *node) const {
        if (!node) return nullptr;
        while (node->right) {
            node = node->right;
        }
        return node;
    }

    // Helper: get next node (for iterator++)
    Node* getNext(Node *node) const {
        if (!node) return nullptr;
        if (node->right) {
            return findLeftmost(node->right);
        }
        Node *p = node->parent;
        while (p && node == p->right) {
            node = p;
            p = p->parent;
        }
        return p;
    }

    // Helper: get previous node (for iterator--)
    Node* getPrev(Node *node) const {
        if (!node) return rightmost_node;
        if (node->left) {
            return findRightmost(node->left);
        }
        Node *p = node->parent;
        while (p && node == p->left) {
            node = p;
            p = p->parent;
        }
        return p;
    }

    // Red-Black Tree operations
    void rotateLeft(Node *x) {
        Node *y = x->right;
        x->right = y->left;
        if (y->left) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (!x->parent) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    void rotateRight(Node *x) {
        Node *y = x->left;
        x->left = y->right;
        if (y->right) {
            y->right->parent = x;
        }
        y->parent = x->parent;
        if (!x->parent) {
            root = y;
        } else if (x == x->parent->right) {
            x->parent->right = y;
        } else {
            x->parent->left = y;
        }
        y->right = x;
        x->parent = y;
    }

    void fixInsert(Node *z) {
        while (z->parent && z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node *y = z->parent->parent->right;
                if (y && y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        rotateLeft(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotateRight(z->parent->parent);
                }
            } else {
                Node *y = z->parent->parent->left;
                if (y && y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rotateRight(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotateLeft(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    void transplant(Node *u, Node *v) {
        if (!u->parent) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        if (v) {
            v->parent = u->parent;
        }
    }

    void fixDelete(Node *x, Node *x_parent) {
        while (x != root && (!x || x->color == BLACK)) {
            if (x == x_parent->left) {
                Node *w = x_parent->right;
                if (w->color == RED) {
                    w->color = BLACK;
                    x_parent->color = RED;
                    rotateLeft(x_parent);
                    w = x_parent->right;
                }
                if ((!w->left || w->left->color == BLACK) &&
                    (!w->right || w->right->color == BLACK)) {
                    w->color = RED;
                    x = x_parent;
                    x_parent = x->parent;
                } else {
                    if (!w->right || w->right->color == BLACK) {
                        if (w->left) w->left->color = BLACK;
                        w->color = RED;
                        rotateRight(w);
                        w = x_parent->right;
                    }
                    w->color = x_parent->color;
                    x_parent->color = BLACK;
                    if (w->right) w->right->color = BLACK;
                    rotateLeft(x_parent);
                    x = root;
                    break;
                }
            } else {
                Node *w = x_parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x_parent->color = RED;
                    rotateRight(x_parent);
                    w = x_parent->left;
                }
                if ((!w->right || w->right->color == BLACK) &&
                    (!w->left || w->left->color == BLACK)) {
                    w->color = RED;
                    x = x_parent;
                    x_parent = x->parent;
                } else {
                    if (!w->left || w->left->color == BLACK) {
                        if (w->right) w->right->color = BLACK;
                        w->color = RED;
                        rotateLeft(w);
                        w = x_parent->left;
                    }
                    w->color = x_parent->color;
                    x_parent->color = BLACK;
                    if (w->left) w->left->color = BLACK;
                    rotateRight(x_parent);
                    x = root;
                    break;
                }
            }
        }
        if (x) x->color = BLACK;
    }

    void deleteNode(Node *z) {
        Node *y = z;
        Node *x, *x_parent;
        Color y_original_color = y->color;

        if (!z->left) {
            x = z->right;
            x_parent = z->parent;
            transplant(z, z->right);
        } else if (!z->right) {
            x = z->left;
            x_parent = z->parent;
            transplant(z, z->left);
        } else {
            y = findLeftmost(z->right);
            y_original_color = y->color;
            x = y->right;
            if (y->parent == z) {
                x_parent = y;
            } else {
                x_parent = y->parent;
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        delete z;

        if (y_original_color == BLACK) {
            fixDelete(x, x_parent);
        }
    }

    void clearTree(Node *node) {
        if (!node) return;
        clearTree(node->left);
        clearTree(node->right);
        delete node;
    }

    Node* copyTree(Node *node, Node *parent) {
        if (!node) return nullptr;
        Node *new_node = new Node(*(node->data), parent);
        new_node->color = node->color;
        new_node->left = copyTree(node->left, new_node);
        new_node->right = copyTree(node->right, new_node);
        return new_node;
    }

public:
    class const_iterator;
    class iterator {
        friend class map;
        friend class const_iterator;
    private:
        const map *map_ptr;
        Node *node_ptr;

        iterator(const map *m, Node *n) : map_ptr(m), node_ptr(n) {}

    public:
        iterator() : map_ptr(nullptr), node_ptr(nullptr) {}

        iterator(const iterator &other) : map_ptr(other.map_ptr), node_ptr(other.node_ptr) {}

        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        iterator& operator++() {
            if (!node_ptr) {
                throw invalid_iterator();
            }
            node_ptr = map_ptr->getNext(node_ptr);
            return *this;
        }

        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        iterator& operator--() {
            if (!map_ptr) {
                throw invalid_iterator();
            }
            if (!node_ptr) {
                node_ptr = map_ptr->rightmost_node;
                if (!node_ptr) {
                    throw invalid_iterator();
                }
            } else {
                node_ptr = map_ptr->getPrev(node_ptr);
                if (!node_ptr) {
                    throw invalid_iterator();
                }
            }
            return *this;
        }

        value_type& operator*() const {
            if (!node_ptr) {
                throw invalid_iterator();
            }
            return *(node_ptr->data);
        }

        bool operator==(const iterator &rhs) const {
            return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
        }

        bool operator==(const const_iterator &rhs) const {
            return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
        }

        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }

        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }

        value_type* operator->() const noexcept {
            return node_ptr->data;
        }
    };

    class const_iterator {
        friend class map;
        friend class iterator;
    private:
        const map *map_ptr;
        Node *node_ptr;

        const_iterator(const map *m, Node *n) : map_ptr(m), node_ptr(n) {}

    public:
        const_iterator() : map_ptr(nullptr), node_ptr(nullptr) {}

        const_iterator(const const_iterator &other) : map_ptr(other.map_ptr), node_ptr(other.node_ptr) {}

        const_iterator(const iterator &other) : map_ptr(other.map_ptr), node_ptr(other.node_ptr) {}

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        const_iterator& operator++() {
            if (!node_ptr) {
                throw invalid_iterator();
            }
            node_ptr = map_ptr->getNext(node_ptr);
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }

        const_iterator& operator--() {
            if (!map_ptr) {
                throw invalid_iterator();
            }
            if (!node_ptr) {
                node_ptr = map_ptr->rightmost_node;
                if (!node_ptr) {
                    throw invalid_iterator();
                }
            } else {
                node_ptr = map_ptr->getPrev(node_ptr);
                if (!node_ptr) {
                    throw invalid_iterator();
                }
            }
            return *this;
        }

        const value_type& operator*() const {
            if (!node_ptr) {
                throw invalid_iterator();
            }
            return *(node_ptr->data);
        }

        bool operator==(const iterator &rhs) const {
            return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
        }

        bool operator==(const const_iterator &rhs) const {
            return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
        }

        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }

        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }

        const value_type* operator->() const noexcept {
            return node_ptr->data;
        }
    };

    map() : root(nullptr), leftmost_node(nullptr), rightmost_node(nullptr), node_count(0) {}

    map(const map &other) : root(nullptr), leftmost_node(nullptr), rightmost_node(nullptr), node_count(0) {
        if (other.root) {
            root = copyTree(other.root, nullptr);
            leftmost_node = findLeftmost(root);
            rightmost_node = findRightmost(root);
            node_count = other.node_count;
        }
    }

    map& operator=(const map &other) {
        if (this != &other) {
            clear();
            if (other.root) {
                root = copyTree(other.root, nullptr);
                leftmost_node = findLeftmost(root);
                rightmost_node = findRightmost(root);
                node_count = other.node_count;
            }
        }
        return *this;
    }

    ~map() {
        clear();
    }

    T& at(const Key &key) {
        Node *node = findNode(key);
        if (!node) {
            throw index_out_of_bound();
        }
        return node->data->second;
    }

    const T& at(const Key &key) const {
        Node *node = findNode(key);
        if (!node) {
            throw index_out_of_bound();
        }
        return node->data->second;
    }

    T& operator[](const Key &key) {
        Node *node = findNode(key);
        if (node) {
            return node->data->second;
        }

        // Insert new element with default value
        value_type val(key, T());
        auto result = insert(val);
        return result.first->second;
    }

    const T& operator[](const Key &key) const {
        Node *node = findNode(key);
        if (!node) {
            throw index_out_of_bound();
        }
        return node->data->second;
    }

    iterator begin() {
        return iterator(this, leftmost_node);
    }

    const_iterator cbegin() const {
        return const_iterator(this, leftmost_node);
    }

    iterator end() {
        return iterator(this, nullptr);
    }

    const_iterator cend() const {
        return const_iterator(this, nullptr);
    }

    bool empty() const {
        return node_count == 0;
    }

    size_t size() const {
        return node_count;
    }

    void clear() {
        clearTree(root);
        root = nullptr;
        leftmost_node = nullptr;
        rightmost_node = nullptr;
        node_count = 0;
    }

    pair<iterator, bool> insert(const value_type &value) {
        // Find insertion point
        Node *parent = nullptr;
        Node *curr = root;

        while (curr) {
            parent = curr;
            if (comp(value.first, curr->data->first)) {
                curr = curr->left;
            } else if (comp(curr->data->first, value.first)) {
                curr = curr->right;
            } else {
                // Key already exists
                return pair<iterator, bool>(iterator(this, curr), false);
            }
        }

        // Create new node
        Node *new_node = new Node(value, parent);
        node_count++;

        // Insert node
        if (!parent) {
            root = new_node;
            leftmost_node = new_node;
            rightmost_node = new_node;
        } else if (comp(value.first, parent->data->first)) {
            parent->left = new_node;
            if (parent == leftmost_node) {
                leftmost_node = new_node;
            }
        } else {
            parent->right = new_node;
            if (parent == rightmost_node) {
                rightmost_node = new_node;
            }
        }

        // Fix Red-Black Tree properties
        fixInsert(new_node);

        return pair<iterator, bool>(iterator(this, new_node), true);
    }

    void erase(iterator pos) {
        if (!pos.node_ptr || pos.map_ptr != this) {
            throw invalid_iterator();
        }

        Node *to_delete = pos.node_ptr;

        // Update leftmost and rightmost if needed
        if (to_delete == leftmost_node) {
            leftmost_node = getNext(to_delete);
        }
        if (to_delete == rightmost_node) {
            rightmost_node = getPrev(to_delete);
        }

        deleteNode(to_delete);
        node_count--;
    }

    size_t count(const Key &key) const {
        return findNode(key) ? 1 : 0;
    }

    iterator find(const Key &key) {
        Node *node = findNode(key);
        return iterator(this, node);
    }

    const_iterator find(const Key &key) const {
        Node *node = findNode(key);
        return const_iterator(this, node);
    }
};

}

#endif
