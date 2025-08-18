#ifndef BSP_TREE_H
#define BSP_TREE_H

#include "DungeonCommon.h"

struct BSPNode {
    Rect bounds;
    BSPNode* left = nullptr;
    BSPNode* right = nullptr;
    Rect room;
    bool hasRoom = false;

    BSPNode(const Rect& bounds);
    ~BSPNode();
    bool isLeaf() const;
};

class BSPTree {
public:
    BSPTree();
    ~BSPTree();
    void buildTree(const Rect& rootArea, int maxDepth, int targetRooms);
    void createRooms(std::vector<Rect>& rooms);
    BSPNode* getRoot() const { return root; }
    
private:
    BSPNode* root = nullptr;
    std::mt19937 rng{std::random_device{}()};

    void splitNode(BSPNode* node, int depth, int minSize, int targetRooms);
    int countLeafNodes(BSPNode* node) const;
    void createRoomInNode(BSPNode* node);
};

#endif