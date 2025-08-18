#include "include/world/BSPTree.h"
#include <functional>
#include <algorithm>

BSPNode::BSPNode(const Rect& bounds) 
    : bounds(bounds), left(nullptr), right(nullptr), hasRoom(false) {}

BSPNode::~BSPNode() {
    delete left;
    delete right;
}

bool BSPNode::isLeaf() const {
    return !left && !right;
}

BSPTree::BSPTree() : root(nullptr) {}

BSPTree::~BSPTree() {
    delete root;
}

void BSPTree::buildTree(const Rect& rootArea, int maxDepth, int targetRooms) {
    delete root;
    root = new BSPNode(rootArea);
    int minSize = (targetRooms > 32) ? 16 : 
                 (targetRooms < 8) ? 28 : 18;
    splitNode(root, maxDepth, minSize, targetRooms);
}

void BSPTree::splitNode(BSPNode* node, int depth, int minSize, int targetRooms) {
    if (depth <= 0 || 
        node->bounds.w < minSize || 
        node->bounds.h < minSize) return;

    // Determine split direction
    float ratio = static_cast<float>(node->bounds.w) / node->bounds.h;
    bool splitHorizontal;
    if (ratio > 1.25f) splitHorizontal = false;
    else if (ratio < 0.8f) splitHorizontal = true;
    else splitHorizontal = (rand() % 2 == 0);

    // Calculate split position
    int splitPos;
    int minSplit = minSize / 2 + 2;
    
    if (splitHorizontal) {
        int minY = node->bounds.y + minSplit;
        int maxY = node->bounds.y + node->bounds.h - minSplit;
        if (minY >= maxY) return;
        splitPos = minY + rand() % (maxY - minY);
        
        node->left = new BSPNode(Rect(
            node->bounds.x, node->bounds.y,
            node->bounds.w, splitPos - node->bounds.y
        ));
        node->right = new BSPNode(Rect(
            node->bounds.x, splitPos,
            node->bounds.w, node->bounds.y + node->bounds.h - splitPos
        ));
    } else {
        int minX = node->bounds.x + minSplit;
        int maxX = node->bounds.x + node->bounds.w - minSplit;
        if (minX >= maxX) return;
        splitPos = minX + rand() % (maxX - minX);
        
        node->left = new BSPNode(Rect(
            node->bounds.x, node->bounds.y,
            splitPos - node->bounds.x, node->bounds.h
        ));
        node->right = new BSPNode(Rect(
            splitPos, node->bounds.y,
            node->bounds.x + node->bounds.w - splitPos, node->bounds.h
        ));
    }

    splitNode(node->left, depth - 1, minSize, targetRooms);
    splitNode(node->right, depth - 1, minSize, targetRooms);
}

int BSPTree::countLeafNodes(BSPNode* node) const {
    if (!node) return 0;
    if (node->isLeaf()) return 1;
    return countLeafNodes(node->left) + countLeafNodes(node->right);
}

void BSPTree::createRooms(std::vector<Rect>& rooms) {
    std::function<void(BSPNode*)> create = [&](BSPNode* node) {
        if (!node) return;
        
        if (node->isLeaf()) {
            // Calculate room dimensions
            int minDim = 6, maxDim = 18;
            int roomW = std::min(maxDim, node->bounds.w - 2);
            int roomH = std::min(maxDim, node->bounds.h - 2);
            roomW = std::max(minDim, roomW);
            roomH = std::max(minDim, roomH);
            
            // Position room within node bounds
            int offsetX = 1 + rand() % (node->bounds.w - roomW - 1);
            int offsetY = 1 + rand() % (node->bounds.h - roomH - 1);
            
            node->room = Rect(
                node->bounds.x + offsetX,
                node->bounds.y + offsetY,
                roomW,
                roomH
            );
            node->hasRoom = true;
            rooms.push_back(node->room);
        } else {
            create(node->left);
            create(node->right);
        }
    };
    
    create(root);
}