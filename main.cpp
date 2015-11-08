#include <iostream>
#include <vector>
#include "TernaryTree.h"

using namespace std;

int main() {
    TernaryTree<string, int> treeS;
    TernaryTree<vector<int>, int> treeV;

    treeV.insert(vector<int>({0,1,2,3}));
    treeV.insert(vector<int>({1,2,3,4}));

    treeS.insert("LOL");
    treeS.insert("ASSDSAD");

    cout << treeV.search(vector<int>({0,1,2,3})) << endl;
    cout << treeV.search(vector<int>({1,2,3})) << endl;

    cout << treeS.search("AS") << endl;
    cout << treeS.search("LOL") << endl;

    return 0;
}