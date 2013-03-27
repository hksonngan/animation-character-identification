#ifndef _SEGMENTATIONGRAPH_HPP_
#define _SEGMENTATIONGRAPH_HPP_

#include <map>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "LabeledGraph.hpp"
#include "DisjointSet.hpp"

using namespace cv;
using namespace std;

/**
 * Computes the segmentation graph of an image segmentation, where
 * vertices are segments and there is an edge between two segments
 * iff these segments are connected in a 4-connexity sense. Every edge
 * is represented in only one adjacency list.
 *
 * @param image the segmented image
 * @param segmentation a segmentation of the image
 * @param grid grid graph of the image
 * @return the segmentation graph of this image, with empty labels
 * (the default constructor is called).
 */
template < typename T >
LabeledGraph<T> segmentationGraph(Mat_<Vec<uchar,3> > &image, DisjointSetForest &segmentation, WeightedGraph &grid) {
  int numberOfComponents = segmentation.getNumberOfComponents();
  LabeledGraph<T> graph(numberOfComponents);
  vector<vector<bool> > adjMatrix(numberOfComponents, vector<bool>(numberOfComponents, false));
  map<int,int> rootIndexes = segmentation.getRootIndexes();

  // for each pair of neighboring pixels
  for (int i = 0; i < (int)grid.getEdges().size(); i++) {
    Edge edge = grid.getEdges()[i];
    int srcRoot = rootIndexes[segmentation.find(edge.source)];
    int dstRoot = rootIndexes[segmentation.find(edge.destination)];

    // if they are not in the same segment and there isn't
    // already an edge between them, add one.
    if (srcRoot != dstRoot && 
	!adjMatrix[srcRoot][dstRoot] &&
	!adjMatrix[dstRoot][srcRoot]) {
      adjMatrix[srcRoot][dstRoot] = true;
	  adjMatrix[dstRoot][srcRoot] = true;
      graph.addEdge(srcRoot,dstRoot,0);
    }
  }

  return graph;
}

vector<Vec<float,2> > segmentCenters(Mat_<Vec<uchar,3> > &image, DisjointSetForest &segmentation);

#endif
