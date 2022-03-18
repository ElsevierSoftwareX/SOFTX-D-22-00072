# ifndef PLOTTILE_HPP_
# define PLOTTILE_HPP_

#include "VPQTreeNodeS.hpp"


using namespace std;


VPQTreeNodeS * locateTileNode(int z, int x, int y, VPQTreeNodeS *seek);

void pointUpResolutionSample(int z, int x, int y, VPQTreeNodeS *tile_node, VPQTreeNodeS *pointVPQTree, char *tile_area);

void lineUpResolutionSample(int z, int x, int y, VPQTreeNodeS *tile_node, VPQTreeNodeS *lineVPQTree, char *tile_area);

void polygonUpResolutionSample(int z, int x, int y, VPQTreeNodeS *polygonVPQTree, char *tile_area);


# endif