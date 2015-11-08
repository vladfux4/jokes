//
// Created by vladfux on 11/8/15.
//

#ifndef JOKE_TERNARYNODE_H
#define JOKE_TERNARYNODE_H

template<class ElementType, typename IdType>
class TernaryNode {
private:
    ElementType element = 0;
public:
    IdType id = 0;
    TernaryNode *leftChild = nullptr;
    TernaryNode *middleChild = nullptr;
    TernaryNode *rightChild = nullptr;

    char getElement() { return element; };

    TernaryNode(const char element) : element(element) {}
    ~TernaryNode() {
        if (leftChild) delete leftChild;
        if (middleChild) delete middleChild;
        if (rightChild) delete rightChild;
    }
};


#endif //JOKE_TERNARYNODE_H
