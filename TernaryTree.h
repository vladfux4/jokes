//
// Created by vladfux on 11/8/15.
//

#ifndef JOKE_TERNARYTREE_H
#define JOKE_TERNARYTREE_H

#include "TernaryNode.h"

template<class ArrayType, typename IdType>
class TernaryTree {
private:
    IdType id = 0;
    TernaryNode<typename ArrayType::value_type, IdType> rootNode;
public:
    TernaryTree() : rootNode(TernaryNode<typename ArrayType::value_type, IdType>(0)) { }

    void insert(const ArrayType array);
    IdType search(const ArrayType array);
};

#include "TernaryTree.tpp"

#endif //JOKE_TERNARYTREE_H
