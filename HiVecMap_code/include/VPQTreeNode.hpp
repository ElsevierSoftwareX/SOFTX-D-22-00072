/*
// VPQtree node class, the root node represents the VPQtree
// strategies: Nodes are dynamically allocated and deleted during insertion, VPQTree is not a full quadtree.
*/

#include <string>
#include <iostream>
#include <fstream>

#ifndef VPQTREENODE_HPP_
#define VPQTREENODE_HPP_

using namespace std;

extern short tile_level;
extern short tqTree_level;

// 四叉树类型枚举
enum QuadType{
    ROOT,
    UP_LEFT,
    UP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
};

class VPQTreeNode{
    public:
        // constructor and destructor
        VPQTreeNode(float _x, float _y, float _width, short _level, QuadType _quadType, VPQTreeNode * _parent);
        ~VPQTreeNode();
    public:
        bool IsContain(float node_x, float node_y, float width, float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max) const;
        bool IsContainPoint(float node_x, float node_y, float width, float point_x, float point_y) const;
        bool IsIntersect(float node_x, float node_y, float width, float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max) const;
        bool IsCoverNode(float node_x, float node_y, float width, float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max, string obj_type) const;

        void InsertPointObject(float point_x, float point_y);
        void InsertLineObject(float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max, string obj_type); 
        void InsertPolygonSegment(float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max, string obj_type);
        void InsertPolygonBox(float box_x_min, float box_y_min, float box_x_max, float box_y_max, float ring_x[], float ring_y[], int len);
        void saveToFile(ofstream &outFile);
        void saveToFile_polygon(ofstream &outFile);

        VPQTreeNode * buildMBNode(float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max);
        VPQTreeNode * buildMBNode_polygon(float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max);
        
    public:
        VPQTreeNode *parent;
        VPQTreeNode *upLeftNode;
        VPQTreeNode *upRightNode;
        VPQTreeNode *bottomLeftNode;
        VPQTreeNode *bottomRightNode;
        QuadType quadType;              
        float x;                       
        float y;
        float width;
        short level;    
        char polygon_type;
        
};



class nodeInfo{
    public:
        short level;
        short quadType;
    public:
        nodeInfo(short _level, short _quadType){
            this->level = _level;
            this->quadType = _quadType;
        }
        
};


class nodeInfoPolygon{
    public:
        short level;
        short quadType;
        char polygon_type;
    public:
        nodeInfoPolygon(short _level, short _quadType, char _polygon_type){
            this->level = _level;
            this->quadType = _quadType;
            this->polygon_type = _polygon_type;
        }
        
};

# endif