#include "PreProcessing.h"

#define DEBUG_PREPROCESSING false

static double constOne(const Mat& m1, const Mat& m2) {
	return 1;
}

static void removeSmallComponents(const Mat_<float> &mask, Mat_<float> &connectedMask) {
	Mat_<Vec3b> dummy(mask.rows, mask.cols);
	WeightedGraph grid = gridGraph(dummy, CONNECTIVITY_4, mask, constOne, true);
	vector<int> inCC;
	int nbCC;
	vector<WeightedGraph> components;
	vector<int> vertexIdx;

	connectedComponents(grid, inCC, &nbCC);
	inducedSubgraphs(grid, inCC, nbCC, vertexIdx, components);
	int largestIndex = max_element(components.begin(), components.end(), compareGraphSize) - components.begin();

	connectedMask = Mat_<float>(mask.rows, mask.cols);

	for (int i = 0; i < mask.rows; i++) {
		for (int j = 0; j < mask.cols; j++) {
			if (inCC[toRowMajor(mask.cols, j, i)] == largestIndex) {
				connectedMask(i,j) = 1;
			} else {
				connectedMask(i,j) = 0;
			}
		}
	}
}

void preProcessing(const Mat_<Vec3b> &rawImage, const Mat_<float> &rawMask, Mat_<Vec3b> &processedImage, Mat_<float> &processedMask, int kuwaharaHalfsize, int maxNbPixels) {
	assert(kuwaharaHalfsize <= (numeric_limits<uchar>::max() - 1) / 2);

	Mat_<Vec3b> resized;
	Mat_<float> resizedMask;

	resizeImage(rawImage, rawMask, resized, resizedMask, maxNbPixels);

	removeSmallComponents(resizedMask, processedMask);

	Mat_<Vec3b> equalized;
	
	equalizeColorHistogram(resized, processedMask, equalized);

	Mat_<Vec3b> filtered;

	KuwaharaFilter(equalized, filtered, 2 * kuwaharaHalfsize + 1);

	cvtColor(filtered, processedImage, CV_BGR2HSV);

	if (DEBUG_PREPROCESSING) {
		imshow("original", rawImage);
		imshow("original mask", rawMask);
		imshow("resized", resized);
		imshow("resized mask", resizedMask);
		imshow("connected mask", processedMask);
		waitKey(0);
	}
}
