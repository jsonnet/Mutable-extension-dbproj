#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <utility>


template<
    typename Key,
    typename Value,
    typename Compare = std::less<Key>>
struct BPlusTree
{
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using size_type = std::size_t;
    using key_compare = Compare;

    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    private:
    using entry_type = std::pair<Key, Value>;

    public:
    struct entry_comparator
    {
        bool operator()(const value_type &first, const value_type &second) const {
            key_compare c;
            return c(first.first, second.first);
        }
    };

    struct inner_node;
    struct leaf_node;

    private:
    /* TODO: 2.1.4.1
     *
     * Declare fields of the B+-tree.
     */


    /*--- Iterator ---------------------------------------------------------------------------------------------------*/
    private:
    template<bool C>
    struct the_iterator
    {
        friend struct BPlusTree;

        static constexpr bool Is_Const = C;
        using pointer_type = std::conditional_t<Is_Const, const_pointer, pointer>;
        using reference_type = std::conditional_t<Is_Const, const_reference, reference>;

        private:
        leaf_node *node_; ///< the current leaf node
        entry_type *elem_; ///< the current element in the current leaf

        public:
        the_iterator(leaf_node *node, entry_type *elem)
            : node_(node)
            , elem_(elem)
        { }

        /** Returns true iff this iterator points to the same entry as `other`. */
        bool operator==(the_iterator other) const { return this->elem_ == other.elem_; }
        /** Returns false iff this iterator points to the same entry as `other`. */
        bool operator!=(the_iterator other) const { return not operator==(other); }

        /** Advances the iterator to the next element.
         *
         * @return this iterator
         */
        the_iterator & operator++() {
            /* TODO: 2.1.2.4
             * Advance this iterator to the next entry.  The behaviour is undefined if no next entry exists.
             */
            assert(false && "not implemented");
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
    struct the_leaf_iterator
    {
        friend struct BPlusTree;

        static constexpr bool Is_Const = C;
        using pointer_type = std::conditional_t<Is_Const, const leaf_node*, leaf_node*>;
        using referene_type = std::conditional_t<Is_Const, const leaf_node&, leaf_node&>;

        private:
        pointer_type node_; ///< the current leaf node

        public:
        the_leaf_iterator(pointer_type node) : node_(node) { }

        /** Returns true iff this iterator points to the same leaf as `other`. */
        bool operator==(the_leaf_iterator other) const { return this->node_ == other.node_; }
        /** Returns false iff this iterator points to the same leaf as `other`. */
        bool operator!=(the_leaf_iterator other) const { return not operator==(other); }

        /** Advances the iterator to the next leaf.
         *
         * @return this iterator
         */
        the_leaf_iterator & operator++() {
            /* TODO: 2.1.2.4
             * Advance this iterator to the next leaf.  The behaviour is undefined if no next leaf exists.
             */
            assert(false && "not implemented");
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
        referene_type operator*() const { return *node_; }
    };
    public:
    using leaf_iterator = the_leaf_iterator<false>;
    using const_leaf_iterator = the_leaf_iterator<true>;

    /*--- Range Type -------------------------------------------------------------------------------------------------*/
    private:
    template<bool C>
    struct the_range
    {
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
    /** Implements an inner node in a B+-Tree.  An inner node stores k-1 keys that distinguish the k child pointers. */
    struct inner_node
    {
        private:
        static constexpr size_type COMPUTE_CAPACITY() {
            /* TODO: 2.1.3.2
             * Compute the capacity of a inner nodes.  The capacity is the number of children an inner node can contain.
             * This means, the capacity equals the fan out.  If the capacity is *n*, the inner node can contain *n*
             * children and *n - 1* keys.
             * When computing the capacity of a inner nodes, consider *all fields and padding* in your computation.  For
             * *fundamental types*, the size equals the alignment requirement of that type.
             */
            return 0;
        }

        private:
        /* TODO: 2.1.3.1
         * Declare the fields of an inner node.
         */

        public:
        /** Returns the number of children. */
        size_type size() const {
            /* TODO: 2.1.3.3 */
            assert(false && "not implemented");
        }
        /** Returns true iff the inner node is full, i.e. capacity is reached. */
        bool full() const {
            /* TODO: 2.1.3.3 */
            assert(false && "not implemented");
        }
    };

    /** Implements a leaf node in a B+-Tree.  A leaf node stores key-value-pairs.  */
    struct leaf_node
    {
        private:
        static constexpr size_type COMPUTE_CAPACITY() {
            /* TODO: 2.1.2.2
             * Compute the capacity of leaf nodes.  The capacity is the number of key-value-pairs a leaf node can
             * contain.  If the capacity is *n*, the leaf node can contain *n* key-value-pairs.
             * When computing the capacity of inner nodes, consider *all fields and padding* in your computation.  Use
             * `sizeof` to get the size of a field.  For *fundamental types*, the size equals the alignment requirement
             * of that type.
             */
            return 0;
        }

        private:
        /* TODO: 2.1.2.1
         * Declare the fields of a leaf node.
         */

        public:
        /** Returns the number of entries. */
        size_type size() const {
            /* TODO: 2.1.2.3 */
            assert(false && "not implemented");
        }
        /** Returns true iff the leaf is empty, i.e. has zero entries. */
        bool empty() const {
            /* TODO: 2.1.2.3 */
            assert(false && "not implemented");
        }
        /** Returns true iff the leaf is full, i.e. the capacity is reached. */
        bool full() const {
            /* TODO: 2.1.2.3 */
            assert(false && "not implemented");
        }
        /** Returns a pointer to the next leaf node in the ISAM or `nullptr` if there is no next leaf node. */
        leaf_node * next() const {
            /* TODO: 2.1.2.3 */
            assert(false && "not implemented");
        }
        /** Sets the pointer to the next leaf node in the ISAM.  Returns the previously set value.
         *
         * @return the previously set next leaf
         */
        leaf_node * next(leaf_node *new_next) {
            /* TODO: 2.1.2.3 */
            assert(false && "not implemented");
        }

        /** Returns an iterator to the first entry in the leaf. */
        entry_type * begin() {
            /* TODO: 2.1.2.4 */
            assert(false && "not implemented");
        }
        /** Returns an iterator to the entry following the last entry in the leaf. */
        entry_type * end() {
            /* TODO: 2.1.2.4 */
            assert(false && "not implemented");
        }
        /** Returns an iterator to the first entry in the leaf. */
        const entry_type * begin() const {
            /* TODO: 2.1.2.4 */
            assert(false && "not implemented");
        }
        /** Returns an iterator to the entry following the last entry in the leaf. */
        const entry_type * end() const {
            /* TODO: 2.1.2.4 */
            assert(false && "not implemented");
        }
        /** Returns an iterator to the first entry in the leaf. */
        const entry_type * cbegin() const {
            /* TODO: 2.1.2.4 */
            assert(false && "not implemented");
        }
        /** Returns an iterator to the entry following the last entry in the leaf. */
        const entry_type * cend() const {
            /* TODO: 2.1.2.4 */
            assert(false && "not implemented");
        }
    };

    /*--- Factory methods --------------------------------------------------------------------------------------------*/
    template<typename It>
    static BPlusTree Bulkload(It begin, It end) {
        /* TODO: 2.1.4.4
         *
         * Bulkload the B+-tree with the values in the range [begin, end).  The iterators of type `It` are *random
         * access iterators*.  The elements being iterated are `std::pair<key_type, mapped_type>`.
         */
        assert(false && "not implemented");
    }

    template<typename Container>
    static BPlusTree Bulkload(const Container &C) {
        using std::begin, std::end;
        return Bulkload(begin(C), end(C));
    }


    /*--- Start of B+-Tree code --------------------------------------------------------------------------------------*/
    private:
    BPlusTree() {
        /* TODO: 2.1.4.1 */
    }

    public:
    BPlusTree(const BPlusTree&) = delete;
    BPlusTree(BPlusTree&&) = default;

    ~BPlusTree() {
        /* TODO: 2.1.4.1 */
        assert(false && "not implemented");
    }

    /** Returns the number of entries. */
    size_type size() const {
        /* TODO: 2.1.4.2 */
        assert(false && "not implemented");
    }
    /** Returns the height of the tree, i.e. the number of edges on the longest path from leaf to root. */
    size_type height() const {
        /* TODO: 2.1.4.2 */
        assert(false && "not implemented");
    }

    /** Returns an iterator to the first entry in the tree. */
    iterator begin() {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the entry following the last entry in the tree. */
    iterator end() {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the first entry in the tree. */
    const_iterator begin() const {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the entry following the last entry in the tree. */
    const_iterator end() const {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the first entry in the tree. */
    const_iterator cbegin() const {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the entry following the last entry in the tree. */
    const_iterator cend() const {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }

    /** Returns an iterator to the first leaf of the tree. */
    leaf_iterator leaves_begin() {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the next leaf after the last leaf of the tree. */
    leaf_iterator leaves_end() {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the first leaf of the tree. */
    const_leaf_iterator leaves_begin() const {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the next leaf after the last leaf of the tree. */
    const_leaf_iterator leaves_end() const {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the first leaf of the tree. */
    const_leaf_iterator cleaves_begin() const {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the next leaf after the last leaf of the tree. */
    const_leaf_iterator cleaves_end() const {
        /* TODO: 2.1.4.3 */
        assert(false && "not implemented");
    }

    /** Returns an iterator to the first entry with a key that equals `key`, or `end()` if no such entry exists. */
    const_iterator find(const key_type key) const {
        /* TODO: 2.1.4.5 */
        assert(false && "not implemented");
    }
    /** Returns an iterator to the first entry with a key that equals `key`, or `end()` if no such entry exists. */
    iterator find(const key_type &key) {
        /* TODO: 2.1.4.5 */
        assert(false && "not implemented");
    }

    /** Returns the range of entries between `lower` (including) and `upper` (excluding). */
    const_range in_range(const key_type &lower, const key_type &upper) const {
        /* TODO: 2.1.4.6 */
        assert(false && "not implemented");
    }
    /** Returns the range of entries between `lower` (including) and `upper` (excluding). */
    range in_range(const key_type &lower, const key_type &upper) {
        /* TODO: 2.1.4.6 */
        assert(false && "not implemented");
    }
};
