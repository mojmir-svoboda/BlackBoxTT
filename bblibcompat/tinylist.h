#pragma once
#include "memory.h"
#include "utils_string.h"
#include <tchar.h>

struct list_node 
{
    list_node * m_next;
    void * m_val; 
};

struct string_node
{
    string_node * m_next;
    TCHAR str[1];
};

#define do_list(_e,_l) \
    for (_e=(_l); _e; _e = _e->m_next)

#define skipUntil(_e, _l, _pred)  \
    for (_e=(_l); (_e) && !(_pred); _e = _e->m_next)

template <typename NodeT>
NodeT * member (NodeT * list, NodeT * e0)
{
    NodeT * l;
    do_list (l, list)
        if (l == e0)
            break;
    return l;
}

template <typename NodeT, typename ValueT>
NodeT * assoc (NodeT * list, ValueT value)
{
    NodeT * l = 0;
    do_list (l, list)
        if (l->m_val == value)
            break;
    return l;
}

template <typename NodeT>
NodeT ** member_ptr (NodeT ** a0, NodeT * e0)
{
    NodeT *l = 0;
    for (NodeT ** ll = a0 ; NULL != (l = *ll); ll = &l->m_next)
        if (l == e0)
            return ll;
    return NULL;
}

template <typename NodeT>
NodeT ** assoc_ptr (NodeT ** a0, void * val)
{
    NodeT * l = 0;
    for (NodeT ** ll = a0; NULL != (l = *ll); ll = &l->m_next)
        if (l->m_val == val)
            return ll;
    return NULL;
}

template <typename NodeT>
int remove_assoc (NodeT ** a, void * val)
{
    NodeT * q = 0;
    if (NodeT ** pp = assoc_ptr(a, val))
    {
        q = *pp, *pp = q->m_next, m_free(q);
        return 1;
    }
    return 0;
}

template <typename NodeT>
int remove_node (NodeT ** a, NodeT * e)
{
    NodeT * q = 0;
    if (NodeT **pp = member_ptr(a, e))
    {
        q = *pp, *pp = q->m_next;
        return 1;
    }
    return 0;
}

template <typename NodeT>
int remove_item (NodeT ** a, NodeT * e)
{
    int r = remove_node(a, e);
    m_free(e); // TODO what if r == 0 ?
    return r;
}

template <typename NodeT>
void reverse_list (NodeT ** d)
{
    NodeT *a = *d, *b = 0, *c = 0;
    for (b = 0; a; c=a->m_next, a->m_next=b, b=a, a=c)
        ;
    *d = b;
}

template <typename NodeT>
void append_node (NodeT ** a0, NodeT * e0)
{
    NodeT *l, **pp = a0, *e = e0;
    for ( ; NULL != (l = *pp); pp = &l->m_next)
        ;
    *pp = e, e->m_next=NULL;
}

template <typename NodeT>
void cons_node (NodeT ** a0, NodeT * e0)
{
    NodeT **pp = a0, *e = e0;
    e->m_next = *pp, *pp = e;
}

template <typename NodeT, typename ValueT>
NodeT * new_node (ValueT * value)
{
    NodeT * n = static_cast<NodeT *>(m_alloc(sizeof(NodeT)));
    n->m_val = value;
    n->m_next = NULL;
    return n;
}

template <typename NodeT>
NodeT * copy_list (NodeT * l0)
{
    NodeT *p = 0, *l = 0;
    NodeT **pp = &p;
    do_list(l, l0)
        *pp = new_node<NodeT>(l->m_val), pp = &(*pp)->m_next;
    return p;
}

template <typename NodeT>
NodeT * nth_node (NodeT * v0, int n)
{
    NodeT *v = v0;
    while (n && v)  
        n--, v = v->m_next;
    return v;
}

template <typename NodeT>
int listlen (NodeT * v0)
{
    NodeT *v = v0;
    int i = 0;
    while (v) 
        i++, v=v->m_next;
    return i;
}

template <typename NodeT>
void freeall (NodeT ** p)
{
    NodeT *s = 0, *q= *p;
    while (q)
        q = (s = q)->m_next, m_free(s);
    *p = q;
}

/* ------------------------------------------------------------------------- */

inline string_node * new_string_node (TCHAR const * s)
{
    size_t const l = _tcslen(s);
    string_node * b = static_cast<string_node *>(m_alloc(sizeof(string_node) + l * sizeof(TCHAR)));
    tmemcpy(b->str, s, l+1);
    b->m_next = NULL;
    return b;
}

inline void append_string_node (string_node ** p, TCHAR const * s)
{
    append_node(p, new_string_node(s));
}

/* ------------------------------------------------------------------------- */
