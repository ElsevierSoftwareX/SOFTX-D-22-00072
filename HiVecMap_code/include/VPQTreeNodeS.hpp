#include <string>
#include <iostream>
#include <fstream>
#include "VPQTreeNode.hpp"

#ifndef VPQTREENODES_HPP_
#define VPQTREENODES_HPP_

using namespace std;


class VPQTreeNodeS{
    public:
        VPQTreeNodeS(short _level, VPQTreeNodeS * _parent);
        ~VPQTreeNodeS(); 
        void readIndexFile(string index_path);
        void readIndexFile_polygon(string index_path);

    public:
        VPQTreeNodeS *parent;
        VPQTreeNodeS *upLeftNode;
        VPQTreeNodeS *upRightNode;
        VPQTreeNodeS *bottomLeftNode;
        VPQTreeNodeS *bottomRightNode;
        short level;         
        char polygon_type;
        
};

# endif