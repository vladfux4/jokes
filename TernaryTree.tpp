//
// Created by vladfux on 11/8/15.
//

#include "TernaryTree.h"

template<class ArrayType, typename IdType>
void TernaryTree<ArrayType,IdType>::insert(const ArrayType array) {
    TernaryNode<typename ArrayType::value_type, IdType> *pNode = &rootNode;

    for (int i = 0; i < array.size();) {
        if (pNode->getElement() == array[i]) {
            i++;
            if (i < array.size()) {
                if (!pNode->middleChild)
                    pNode->middleChild = new TernaryNode<typename ArrayType::value_type, IdType>(array[i]);
                pNode = pNode->middleChild;
            } else {
                pNode->id = ++id;
            }
        } else if (pNode->getElement() > array[i]) {
            if (!pNode->leftChild)
                pNode->leftChild = new TernaryNode<typename ArrayType::value_type, IdType>(array[i]);
            pNode = pNode->leftChild;
        } else if (pNode->getElement() < array[i]) {
            if (!pNode->rightChild)
                pNode->rightChild = new TernaryNode<typename ArrayType::value_type, IdType>(array[i]);
            pNode = pNode->rightChild;
        }
    }
}

template<class ArrayType, typename IdType>
IdType TernaryTree<ArrayType, IdType>::search(const ArrayType array) {
    TernaryNode<typename ArrayType::value_type, IdType> *pNode = &rootNode;

    for (int i = 0; pNode;) {
        if (pNode->getElement() == array[i]) {
            if (i == array.size() - 1) return pNode->id;
            pNode = pNode->middleChild;
            i++;
        } else if (pNode->getElement() > array[i]) {
            pNode = pNode->leftChild;
        } else if (pNode->getElement() < array[i]) {
            pNode = pNode->rightChild;
        }
    }
    return 0;
}
