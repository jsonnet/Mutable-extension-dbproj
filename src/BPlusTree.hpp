#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <utility>
#include <cmath>
#include <optional>


template<
        typename Key,
        typename Value,
        typename Compare = std::less<Key>>
struct BPlusTree {
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using size_type = std::size_t;
    using key_compare = Compare;

    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;

private:
    using entry_type = std::pair<Key, Value>;

public:
    struct entry_comparator {
        bool operator()(const value_type &first, const value_type &second) const {
            key_compare c;
            return c(first.first, second.first);
        }
    };


    struct inner_node;
    struct leaf_node;

private:
    /*
     *
     * Declare fields of the B+-tree.
     */
    struct tree_node;

    struct inner_node *root;
    size_type numLeaves;
    leaf_node *bottom_left_leaf;
    leaf_node *bottom_right_leaf;

    /*--- Iterator ---------------------------------------------------------------------------------------------------*/
private:
    template<bool C>
    struct the_iterator {
        friend struct BPlusTree;

        static constexpr bool Is_Const = C;
        using pointer_type = std::conditional_t<Is_Const, const_pointer, pointer>;
        using reference_type = std::conditional_t<Is_Const, const_reference, reference>;

    private:
        leaf_node *node_; ///< the current leaf node
        entry_type *elem_; ///< the current element in the current leaf

    public:
        the_iterator(leaf_node *node, entry_type *elem)
                : node_(node), elem_(elem) {}

        /** Returns true iff this iterator points to the same entry as `other`. */
        bool operator==(the_iterator other) const { return this->elem_ == other.elem_; }

        /** Returns false iff this iterator points to the same entry as `other`. */
        bool operator!=(the_iterator other) const { return not operator==(other); }

        /** Advances the iterator to the next element.
         *
         * @return this iterator
         */
        the_iterator &operator++() {

            ++elem_;

            if (node_->end() == elem_) {
                if (node_->next() == nullptr) return *this;

                node_ = node_->next();
                elem_ = node_->begin();
            }

//            else {
//                //normal case: get next element of leaf
//                ++elem_;
//            }

            return *this;
        }

        /** Advances the iterator to the next element.
         *
         * @return this iterator
         */
        the_iterator operator++(int) {
            auto old = *this;
            operator++();
            return old;
        }

        /** Returns a pointer to the designated element. */
        pointer_type operator->() const { return reinterpret_cast<pointer_type>(elem_); }

        /** Returns a reference to the designated element */
        reference_type operator*() const { return *operator->(); }
    };

public:
    using iterator = the_iterator<false>;
    using const_iterator = the_iterator<true>;

private:
    template<bool C>
    struct the_leaf_iterator {
        friend struct BPlusTree;

        static constexpr bool Is_Const = C;
        using pointer_type = std::conditional_t<Is_Const, const leaf_node *, leaf_node *>;
        using reference_type = std::conditional_t<Is_Const, const leaf_node &, leaf_node &>;

    private:
        pointer_type node_; ///< the current leaf node

    public:
        the_leaf_iterator(pointer_type node) : node_(node) {}

        /** Returns true iff this iterator points to the same leaf as `other`. */
        bool operator==(the_leaf_iterator other) const { return this->node_ == other.node_; }

        /** Returns false iff this iterator points to the same leaf as `other`. */
        bool operator!=(the_leaf_iterator other) const { return not operator==(other); }

        /** Advances the iterator to the next leaf.
         *
         * @return this iterator
         */
        the_leaf_iterator &operator++() {
            /*
             * Advance this iterator to the next leaf.  The behaviour is undefined if no next leaf exists.
             */
            node_ = node_->next();

            return *this;
        }

        /** Advances the iterator to the next element.
         *
         * @return this iterator
         */
        the_leaf_iterator operator++(int) {
            auto old = *this;
            operator++();
            return old;
        }

        /** Returns a pointer to the designated leaf. */
        pointer_type operator->() const { return node_; }

        /** Returns a reference to the designated leaf. */
        reference_type operator*() const { return *node_; }
    };

public:
    using leaf_iterator = the_leaf_iterator<false>;
    using const_leaf_iterator = the_leaf_iterator<true>;

    /*--- Range Type -------------------------------------------------------------------------------------------------*/
private:
    template<bool C>
    struct the_range {
        friend struct BPlusTree;

    private:
        the_iterator<C> begin_;
        the_iterator<C> end_;

    public:
        the_range(the_iterator<C> begin, the_iterator<C> end) : begin_(begin), end_(end) {
#ifndef NDEBUG
            const bool is_absolute_end = (end_.node_->next() == nullptr) and (end_.elem_ == end_.node_->end());
            assert(is_absolute_end or not key_compare{}(end_->first, begin_->first)); // begin <= end
#endif
        }

        the_iterator<C> begin() const { return begin_; }

        the_iterator<C> end() const { return end_; }

        bool empty() const { return begin_ == end_; }
    };

public:
    using range = the_range<false>;
    using const_range = the_range<true>;

    /*--- Tree Node Data Types ---------------------------------------------------------------------------------------*/
private:
    struct tree_node {
        virtual ~tree_node() = default;;

        virtual void cleanUP() = 0;

        virtual bool isLeaf() = 0;

        virtual key_type getHighestKey() = 0;

        virtual std::optional<std::tuple<leaf_node *, entry_type *>> find(key_type k) = 0;

        virtual std::optional<std::tuple<leaf_node *, entry_type *>> findFirstInRange(key_type k) = 0;
    };

public:

    /** Implements an inner node in a B+-Tree.  An inner node stores k-1 keys that distinguish the k child pointers. */
    struct inner_node : tree_node {
    private:
        static constexpr size_type COMPUTE_CAPACITY() {
            /*
             * Compute the capacity of a inner nodes.  The capacity is the number of children an inner node can contain.
             * This means, the capacity equals the fan out.  If the capacity is *n*, the inner node can contain *n*
             * children and *n - 1* keys.
             * When computing the capacity of a inner nodes, consider *all fields and padding* in your computation.  For
             * *fundamental types*, the size equals the alignment requirement of that type.
             */

            size_type biggest = sizeof(key_type) > sizeof(size_type) ? sizeof(key_type) : sizeof(size_type);

            size_type capacity = 0;
            //bool found = false;
            while (1 /*!found*/) {
                size_type no_padding = 2*sizeof(size_type) + sizeof(leaf_node *) + sizeof(void *) * capacity +
                                       sizeof(key_type) * (capacity - 1);
                size_type padding = biggest - (no_padding % biggest);

                if ((no_padding + padding) > 64)
                    //found = true;
                    break;
                else
                    capacity++;
            }

            return --capacity;
        }

    private:
        static constexpr size_type computed_capacity = COMPUTE_CAPACITY();
        key_type keys[computed_capacity - 1];
        void *children[computed_capacity]{};  //can either be inner_node* or leaf_node*
        size_type current_capacity = 0;

    public:
        /** Returns the number of children. */
        [[nodiscard]] size_type size() const {
            return current_capacity;
        }

        /** Returns true iff the inner node is full, i.e. capacity is reached. */
        [[nodiscard]] bool full() const {
            return current_capacity == computed_capacity;
        }

        [[nodiscard]] size_type getNumberChildren() const {
            return computed_capacity;
        }

        /* own functions */
        // pop last child element (therefore delete it)
        void *popChild() {
            assert(current_capacity != 0); // Should never be called on a empty node, TODO but what about w/ only 1 elem
            auto tmp = children[--current_capacity];
            children[current_capacity] = nullptr;

            // Also "pop" the child to stop confusion
            if (current_capacity-1 != 0) // we never want to delete the first key, note the node will never be empty
                keys[current_capacity-1] = 0;

            return tmp;
        }

        // insert child by inner_node
        bool insert(inner_node *child) {
            assert(!this->full());
            if (this->full()) return false;

            children[current_capacity] = child;

            //insert key for this child by taking highest value of child: key one will cover )-infinity, highest_value of child 1)
            //second key is for child 2 and 3 where ) highest_value of child 1, highest_value of child 2)

            if (current_capacity < computed_capacity - 1)
                keys[current_capacity] = reinterpret_cast<tree_node *>(children[current_capacity])->getHighestKey();

            current_capacity++;
            return true;
        }

        // Insert child by leaf_node
        bool insert(leaf_node *child) {
            if (this->full()) return false;

            children[current_capacity] = child;

            //insert key for this child by taking highest value of child: key one will cover )-infinity, highest_value of child 1)
            //second key is for child 2 and 3 where ) highest_value of child 1, highest_value of child 2)

            if (current_capacity < computed_capacity - 1)
                keys[current_capacity] = reinterpret_cast<tree_node *>(children[current_capacity])->getHighestKey();

            current_capacity++;
            return true;
        }

        bool insert_front(inner_node *child) {
            if (this->full()) return false;

            //move every children one position to right
            for (size_type i = current_capacity; i > 0; i--) {
                children[i] = children[i - 1];
            }

            //Add children
            children[0] = child;

            //move every key to the right
            for (size_type i = current_capacity-1; i > 0; i--) {
                keys[i] = keys[i - 1];
            }

            //adapt the key
            keys[0] = reinterpret_cast<tree_node *>(children[0])->getHighestKey();

            current_capacity++;

            return true;
        }

        bool insert_front(leaf_node *child) {
            if (this->full()) return false;

            //move every children one position to right
            for (size_type i = current_capacity; i > 0; i--) {
                children[i] = children[i - 1];
            }

            //Add children
            children[0] = child;

            //move every key to the right
            for (size_type i = current_capacity-1; i > 0; i--) {
                keys[i] = keys[i - 1];
            }

            //adapt the key
            keys[0] = reinterpret_cast<tree_node *>(children[0])->getHighestKey();

            current_capacity++;

            return true;
        }

        bool hasBTreeProperty() {
            return this->current_capacity >= ceil(computed_capacity * 1.0 / 2);
        }

        void cleanUP() {
            for (size_type i = 0; i < current_capacity; i++) {
                if (children[i] != nullptr) {
                    reinterpret_cast<tree_node *>(children[i])->cleanUP();

                    if (reinterpret_cast<tree_node *>(children[i])->isLeaf()) {
                        delete (reinterpret_cast<leaf_node *>(children[i]));
                    } else {
                        delete (reinterpret_cast<inner_node *>(children[i]));
                    }
                }
            }
        }

        bool isLeaf() {
            return false;
        }

        /* Returns the heighest key of the subtree*/
        key_type getHighestKey() {
            return reinterpret_cast<tree_node *>(children[current_capacity - 1])->getHighestKey();
        }

        std::optional<std::tuple<leaf_node *, entry_type *>> find(key_type k) {

            if (current_capacity == 0)
                return std::nullopt;

            //Check keys
            for (size_type i = 0; i < current_capacity/* && i < COMPUTE_CAPACITY() - 1*/; i++)
                if (k <= keys[i]) {
                    if (reinterpret_cast<tree_node *>(children[i])->isLeaf())
                        return reinterpret_cast<leaf_node *>(children[i])->find(k);
                    else
                        return reinterpret_cast<inner_node *>(children[i])->find(k);
                }

            // because we now also "pop" the key when popping a child we don't need the following
            //if (!this->full()) return std::nullopt;

            //check if key could be in last leaf in the range of current_capacity
            if (reinterpret_cast<tree_node *>(children[current_capacity - 1])->isLeaf())
                return reinterpret_cast<leaf_node *>(children[current_capacity - 1])->find(k);
            else
                return reinterpret_cast<inner_node *>(children[current_capacity - 1])->find(k);
        }

        std::optional<std::tuple<leaf_node *, entry_type *>> findFirstInRange(key_type k) {
            if (current_capacity == 0)
                return std::nullopt;

            //Check keys
            for (size_type i = 0; i < current_capacity/* && i < COMPUTE_CAPACITY() - 1*/; i++)
                if (k <= keys[i]) {
                    if (reinterpret_cast<tree_node *>(children[i])->isLeaf())
                        return reinterpret_cast<leaf_node *>(children[i])->findFirstInRange(k);
                    else
                        return reinterpret_cast<inner_node *>(children[i])->findFirstInRange(k);
                }

            //if (!this->full()) return std::nullopt;

            //check if key could be in last leaf
            if (reinterpret_cast<tree_node *>(children[current_capacity - 1])->isLeaf())
                return reinterpret_cast<leaf_node *>(children[current_capacity - 1])->findFirstInRange(k);
            else
                return reinterpret_cast<inner_node *>(children[current_capacity - 1])->findFirstInRange(k);
        }

    };

    /** Implements a leaf node in a B+-Tree.  A leaf node stores key-value-pairs.  */
    struct leaf_node : tree_node {
    private:
        static constexpr size_type COMPUTE_CAPACITY() {
            /*
             * Compute the capacity of leaf nodes.  The capacity is the number of key-value-pairs a leaf node can
             * contain.  If the capacity is *n*, the leaf node can contain *n* key-value-pairs.
             * When computing the capacity of inner nodes, consider *all fields and padding* in your computation.  Use
             * `sizeof` to get the size of a field.  For *fundamental types*, the size equals the alignment requirement
             * of that type.
             */
            size_type biggest_tmp = (sizeof(size_type) > sizeof(leaf_node *)) ? sizeof(size_type) : sizeof(leaf_node *);
            size_type biggest = biggest_tmp > sizeof(entry_type) ? biggest_tmp : sizeof(entry_type);

            size_type capacity = 0;
            //bool found = false;
            while (1/*!found*/) {
                size_type no_padding = sizeof(size_type) + sizeof(leaf_node *) + sizeof(entry_type) * capacity;
                size_type padding = biggest - (no_padding % biggest);

                if ((no_padding + padding) > 64)
                    //found = true;
                    break;
                else
                    capacity++;
            }

            //since we stop with the capacity that results in the first bigger struct, return the next lower capacity
            //that has passed the test
            return --capacity;
        }

    private:
        entry_type values[COMPUTE_CAPACITY()];
        size_type num_values = 0;  //Number of values currently contained!
        leaf_node *nextptr = nullptr; //for ISAM

    public:
        /** Returns the number of entries. */
        size_type size() const {
            return num_values;
        }

        /** Returns true iff the leaf is empty, i.e. has zero entries. */
        bool empty() const {
            return num_values == 0;
        }

        /** Returns true iff the leaf is full, i.e. the capacity is reached. */
        bool full() const {
            return num_values == COMPUTE_CAPACITY();
        }

        /** Returns a pointer to the next leaf node in the ISAM or `nullptr` if there is no next leaf node. */
        leaf_node *next() const {
            return nextptr;
        }

        /** Sets the pointer to the next leaf node in the ISAM.  Returns the previously set value.
         *
         * @return the previously set next leaf
         */
        leaf_node *next(leaf_node *new_next) {
            nextptr = new_next;
            return nextptr; //FIXME: return new or old pointer????? read above
        }

        /** Returns an iterator to the first entry in the leaf. */
        entry_type *begin() {
            return (entry_type *) &values[0];
        }

        /** Returns an iterator to the entry following the last entry in the leaf. */
        entry_type *end() {
            return (entry_type *) &values[num_values];
        }

        /** Returns an iterator to the first entry in the leaf. */
        const entry_type *begin() const {
            return (entry_type *) &values[0];
        }

        /** Returns an iterator to the entry following the last entry in the leaf. */
        const entry_type *end() const {
            return (entry_type *) &values[num_values];
        }

        /** Returns an iterator to the first entry in the leaf. */
        const entry_type *cbegin() const {
            return (entry_type *) &values[0];
        }

        /** Returns an iterator to the entry following the last entry in the leaf. */
        const entry_type *cend() const {
            return (entry_type *) &values[num_values];
        }

        /* own functions */
        entry_type popLast() {
            assert(num_values != 0);
            //if (this->empty()) return NULL; // FIXME find right return type

            // No need to delete the value, as we just decrease the pointer for free slots, so new items will overwrite
            return values[--num_values];
        }

        bool insert(entry_type e) {
            if (this->full()) return false;

            values[num_values++] = e;
            return true;
        }

        bool insert_front(entry_type e) {
            if (this->full()) return false;

            //move every children one position to right
            for (size_type i = num_values; i > 0; i--) {
                values[i] = values[i - 1];
            }

            //Add children
            values[0] = e;

            num_values++;
            return true;
        }

        bool hasBTreeProperty() {
            return this->num_values >= ceil(COMPUTE_CAPACITY() * 1.0 / 2);
        }

        void setNextLeaf(leaf_node *_nextptr) {
            nextptr = _nextptr;
        }

        void cleanUP() {
            //Nothing to do
        }

        bool isLeaf() {
            return true;
        }

        key_type getHighestKey() {
            return values[num_values - 1].first;
        }

        std::optional<std::tuple<leaf_node *, entry_type *>> find(key_type k) {
            for (size_type i = 0; i < num_values; i++) {
                if (values[i].first == k)
                    return std::tuple(this, &values[i]);
            }

            return std::nullopt;
        }

        std::optional<std::tuple<leaf_node *, entry_type *>> findFirstInRange(key_type k) {
            for (size_type i = 0; i < num_values; i++) {
                if (values[i].first >= k)
                    return std::tuple(this, &values[i]);
            }

            return std::nullopt;
        }
    };

    /*--- Factory methods --------------------------------------------------------------------------------------------*/
    template<typename It>
    static BPlusTree Bulkload(It begin, It end) {
        /*
         * Bulkload the B+-tree with the values in the range [begin, end).  The iterators of type `It` are *random
         * access iterators*.  The elements being iterated are `std::pair<key_type, mapped_type>`.
         */

        /* create leaves first */
        //int countKeys = std::distance(begin, end);
        //int countLeaves = countKeys / fanout
        auto leaves = std::vector<leaf_node *>();
        leaf_node *prev = nullptr;

        // O(n)
        while (begin != end) {
            leaf_node *newLeaf = new leaf_node();
            if (prev != nullptr) prev->setNextLeaf(newLeaf);
            prev = newLeaf;

            while (!newLeaf->full()) {
                //do not insert when begin==end
                if (begin == end) {
                    //Check if there are new elements to be inserted
                    if (!leaves.empty()) {
                        leaf_node *prev_ = leaves.back();

                        //Ensure that last leaf has BTree property
                        while (!newLeaf->hasBTreeProperty())
                            newLeaf->insert_front(prev_->popLast());
                    }
                    //break while(!newLeaf.full()) else we are stuck on last leaf
                    break;
                } else
                    newLeaf->insert(*begin++);  // fill leaf with key_val pair
            }
            //Add full leaf to output
            leaves.push_back(newLeaf);
        }

        /* handle first level */
        std::vector<inner_node *> outputNodes;

        //O(n)
        //Handle every node of this level
        for (size_t i = 0; i < leaves.size();) {
            auto n = new inner_node();

            while (!n->full() && i < leaves.size())
                n->insert(leaves[i++]);

            outputNodes.push_back(n);
        }

        //O(1)
        //restore BTree property
        if (outputNodes.size() >= 2) {
            //Get last and second last node
            inner_node *n = outputNodes.back();
            inner_node *prev = outputNodes[outputNodes.size() - 2];

            while (!n->hasBTreeProperty())
                n->insert_front(reinterpret_cast<leaf_node *>(prev->popChild()));
        }

        /* insert nodes */
        std::vector<inner_node *> inputNodes;
        inputNodes = outputNodes;
        outputNodes.clear();

        bool finished = outputNodes.size() == 1;

        while (!finished) {
            //Handle every node of this level
            for (size_type i = 0; i < inputNodes.size();) {
                auto n = new inner_node();

                while (!n->full() && i < inputNodes.size())
                    n->insert(inputNodes[i++]);

                outputNodes.push_back(n);
            }

            //ensure BTree property of last node
            if (outputNodes.size() >= 2) {
                auto n = outputNodes.back();
                auto prev_ = outputNodes[outputNodes.size() - 2];

                while (!n->hasBTreeProperty())
                    //n->insert_front(reinterpret_cast<inner_node *>(prev->popChild()));
                    n->insert_front(reinterpret_cast<inner_node *>(prev_->popChild()));
            }

            //If level is finished and only one node is left, it is the root node of the tree --> end building tree here!
            if (outputNodes.size() <= 1)
                finished = true;

            //prepare for next level: output is the new input
            inputNodes = outputNodes;
            outputNodes.clear();
        }

        if (inputNodes.empty())
            return BPlusTree();

        return BPlusTree(inputNodes.front(), leaves.size(), leaves.front(), leaves.back());
    }

    template<typename Container>
    static BPlusTree Bulkload(const Container &C) {
        using std::begin, std::end;
        return Bulkload(begin(C), end(C));
    }


    /*--- Start of B+-Tree code --------------------------------------------------------------------------------------*/


private:
    BPlusTree(inner_node *rootNode, size_type _numLeaves, leaf_node *left, leaf_node *right) {
        root = rootNode;
        numLeaves = _numLeaves;
        bottom_left_leaf = left;
        bottom_right_leaf = right;
    }

    /* Constructor for empty tree */
    BPlusTree() {
        root = new inner_node();
        numLeaves = 0;
        leaf_node *dummy = new leaf_node();
        bottom_right_leaf = dummy;
        bottom_left_leaf = dummy;
    }

public:
    BPlusTree(const BPlusTree &) = delete;

    BPlusTree(BPlusTree &&) = default;

    ~BPlusTree() {
        /* TODO: 2.1.4.1 */

        //root.cleanUP();
        //if root was a pointer initialized with new, also delete
    }

    /** Returns the number of entries. */
    size_type size() const {
        size_type size = 0;

        //Go through every leaf and count the entries
        auto li = cleaves_begin();
        while (li != nullptr) {
            size += li->size();
            li++;
        }

        return size;
    }

    /** Returns the height of the tree, i.e. the number of edges on the longest path from leaf to root. */
    size_type height() const {

        //log(0) and log of a negative number is not defined, so handle this case
        if (numLeaves <= 0 || root->getNumberChildren() <= 0)
            return 0;

        return ceil(log(numLeaves) / log(root->getNumberChildren()));
    }

    /** Returns an iterator to the first entry in the tree. */
    iterator begin() {
        return iterator(bottom_left_leaf, bottom_left_leaf->begin());
    }

    /** Returns an iterator to the entry following the last entry in the tree. */
    iterator end() {
        return iterator(bottom_right_leaf, bottom_right_leaf->end());
    }

    /** Returns an iterator to the first entry in the tree. */
    const_iterator begin() const {
        return const_iterator(bottom_left_leaf, bottom_left_leaf->begin());
    }

    /** Returns an iterator to the entry following the last entry in the tree. */
    const_iterator end() const {
        return const_iterator(bottom_right_leaf, bottom_right_leaf->end());
    }

    /** Returns an iterator to the first entry in the tree. */
    const_iterator cbegin() const {
        return const_iterator(bottom_left_leaf, bottom_left_leaf->begin());
    }

    /** Returns an iterator to the entry following the last entry in the tree. */
    const_iterator cend() const {
        return const_iterator(bottom_right_leaf, bottom_right_leaf->end());
    }

    /** Returns an iterator to the first leaf of the tree. */
    leaf_iterator leaves_begin() {
        return leaf_iterator(bottom_left_leaf);
    }

    /** Returns an iterator to the next leaf after the last leaf of the tree. */
    leaf_iterator leaves_end() {
        return leaf_iterator(bottom_right_leaf->next());
    }

    /** Returns an iterator to the first leaf of the tree. */
    const_leaf_iterator leaves_begin() const {
        return const_leaf_iterator(bottom_left_leaf);
    }

    /** Returns an iterator to the next leaf after the last leaf of the tree. */
    const_leaf_iterator leaves_end() const {
        return const_leaf_iterator(bottom_right_leaf);
    }

    /** Returns an iterator to the first leaf of the tree. */
    const_leaf_iterator cleaves_begin() const {
        return const_leaf_iterator(bottom_left_leaf);
    }

    /** Returns an iterator to the next leaf after the last leaf of the tree. */
    const_leaf_iterator cleaves_end() const {
        return const_leaf_iterator(bottom_right_leaf);
    }

    /** Returns an iterator to the first entry with a key that equals `key`, or `end()` if no such entry exists. */
    const_iterator find(const key_type key) const {
        auto erg = root->find(key);

        if (erg) {
            //successfully found key
            return const_iterator(std::get<0>(*erg), std::get<1>(*erg));
        } else {
            //key not found, return end()
            return cend();
        }
    }

    /** Returns an iterator to the first entry with a key that equals `key`, or `end()` if no such entry exists. */
    iterator find(const key_type &key) {
        auto erg = root->find(key);

        if (erg) {
            //successfully found key
            return iterator(std::get<0>(*erg), std::get<1>(*erg));
        } else {
            //key not found, return end()
            return end();
        }
    }

    /** Returns the range of entries between `lower` (including) and `upper` (excluding). */
    const_range in_range(const key_type &lower, const key_type &upper) const {
        auto erg = root->findFirstInRange(lower);

        if (erg) {
            //save the starting point
            auto beginIt = const_iterator(std::get<0>(*erg), std::get<1>(*erg));

            for (iterator it = const_iterator(std::get<0>(*erg), std::get<1>(*erg)); it != cend(); ++it)
                if (it->first >= upper)
                    return const_range(beginIt, iterator(it.node_, it.elem_));

            return const_range(beginIt, cend());
        } else
            //nothing found
            return const_range(cend(), cend());
    }

    /** Returns the range of entries between `lower` (including) and `upper` (excluding). */
    range in_range(const key_type &lower, const key_type &upper) {
        auto erg = root->findFirstInRange(lower);

        if (erg) {
            //save the starting point
            auto beginIt = iterator(std::get<0>(*erg), std::get<1>(*erg));

            for (iterator it = iterator(std::get<0>(*erg), std::get<1>(*erg)); it != end(); ++it)
                if (it->first >= upper)
                    return range(beginIt, iterator(it.node_, it.elem_));

            return range(beginIt, end());
        } else
            //nothing found
            return range(end(), end());
    }
};
