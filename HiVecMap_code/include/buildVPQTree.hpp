#include "hicore/core.hpp"
#include "hicore/core_creator.hpp"
#include "hicore/core_iterator.hpp"
#include "hicore/shp.hpp"
#include "VPQTreeNode.hpp"

# ifndef BUILDVPQTREE_HPP_
# define BUILDVPQTREE_HPP_

using namespace HiGIS::IO;
using namespace HiGIS::Core;

void coordTran(double &x, double &y);
// build VPQTree of point
VPQTreeNode * buildPointVPQTree(std::string data_path[], int len, std::string file_path);
// build VPQTree of line
VPQTreeNode * buildLineVPQTree(std::string data_path[], int len, std::string file_path);
// build VPQTree of polygon
VPQTreeNode * buildPolygonVPQTree(std::string data_path[], int len, std::string file_path);

# endif