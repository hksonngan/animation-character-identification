#include "Utils.hpp"

int toRowMajor(int width, int x, int y) {
  return x + width * y;
}

int toColumnMajor(int rows, int i, int j) {
	return i + j * rows;
}

pair<int,int> fromRowMajor(int width, int i) {
  pair<int,int> coords(i/width, i%width);

  return coords;
}

static bool isMask(const Mat_<float> &mask) {
	for (int i = 0; i < mask.rows; i++) {
		for (int j = 0; j < mask.cols; j++) {
			if (mask(i,j) != 0 && mask(i,j) != 1) {
				cout<<"failed: mask("<<i<<","<<j<<") = "<<mask(i,j)<<endl;

				return false;
			}
		}
	}

	return true;
}

void loadDataSet(char* folderName, char** charaNames, int nbCharas, int nbImagesPerChara, vector<pair<Mat_<Vec<uchar,3> >,Mat_<float> > > &images, Mat &classes) {
	images = vector<pair<Mat_<Vec<uchar,3> >,Mat_<float> > >(nbCharas * nbImagesPerChara);
	classes = Mat(nbCharas * nbImagesPerChara, 1, CV_32S);

	for (int i = 0; i < nbCharas; i++) {		
		for (int j = 0; j < nbImagesPerChara; j++) {
			char suffix[] = {'_', 'a' + j, '.', 'p', 'n', 'g', '\0'};
			char *fullPath = new char[strlen(folderName) + strlen(charaNames[i]) + strlen(suffix) + 1];
			char maskSuffix[] = {'-', 'm', 'a', 's', 'k', '.', 'p', 'n', 'g', '\0'};
			char *maskPath = new char[strlen(folderName) + strlen(charaNames[i]) + strlen(suffix) + strlen(maskSuffix) + 1];
			
			strcpy(fullPath, folderName);
			strcat(fullPath, charaNames[i]);
			strcat(fullPath, suffix);
			strcpy(maskPath, fullPath);
			strcat(maskPath, maskSuffix);

			int rowMajorIndex = toRowMajor(nbImagesPerChara, j, i);

			Mat_<Vec<uchar, 3> > mask = imread(maskPath);
			vector<Mat_<uchar> > maskChannels;

			split(mask, maskChannels);

			images[rowMajorIndex].first = imread(fullPath);

			Mat_<uchar> thresholdedMask;

			threshold(maskChannels[0], thresholdedMask, 128, 1, THRESH_BINARY_INV);

			images[rowMajorIndex].second = Mat_<float>(thresholdedMask);

			assert(isMask(images[rowMajorIndex].second));
			waitKey(0);
			classes.at<int>(rowMajorIndex,0) = i;

			delete[] fullPath;
			delete[] maskPath;
		}
	}
}

Mat_<double> sparseMul(SparseMat_<double> A, Mat_<double> b) {
	assert(A.size(1) == b.rows);
	assert(b.cols == 1);
	Mat_<double> c = Mat_<double>::zeros(b.rows, 1);

	SparseMatConstIterator_<double> it;

	// iterates over non zero elements
	for (it = A.begin(); it != A.end(); ++it) {
		const SparseMat_<double>::Node* n = it.node();
		int row = n->idx[0];
		int col = n->idx[1];

		c(row, 0) += it.value<double>() * b(col,0);
	}

	return c;
}

bool symmetric(Eigen::SparseMatrix<double> M) {
	for (int k = 0; k < M.outerSize(); k++) {
		for (Eigen::SparseMatrix<double>::InnerIterator it(M, k); it; ++it) {
			if (abs(it.value() - M.coeffRef(it.col(), it.row())) > 0) {
				return false;
			}
		}
	}

	return true;
}

bool positiveDefinite(Eigen::SparseMatrix<double> M) {
	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double> > chol;

	chol.compute(M);

	return chol.info() == Eigen::Success;
}

// Remove a line and column with the same index in a sparse matrix.
void removeLineCol(const Eigen::SparseMatrix<double> &L, int v0, Eigen::SparseMatrix<double> &L0) {
	typedef Eigen::Triplet<double> T;
	vector<T> tripletList;
	tripletList.reserve(L.nonZeros());

	for (int k = 0; k < L.outerSize(); ++k) {
		for (Eigen::SparseMatrix<double>::InnerIterator it(L,k); it; ++it) {
			if (it.row() != v0 && it.col() != v0) {
				int newRow = it.row() < v0 ? it.row() : it.row() - 1;
				int newCol = it.col() < v0 ? it.col() : it.col() - 1;

				tripletList.push_back(T(newRow, newCol, it.value()));
			}
		}
	}

	L0 = Eigen::SparseMatrix<double>(L.rows() - 1, L.cols() - 1);

	L0.setFromTriplets(tripletList.begin(), tripletList.end());
}

int toUpperTriangularPacked(int i, int j) {
	if (i > j) {
		return toUpperTriangularPacked(j, i);
	} else {
		int result = i + (j + 1) * j / 2;

		return result;
	}
}

Mat imHist(Mat hist, float scaleX, float scaleY){
	double maxVal=0;
	minMaxLoc(hist, 0, &maxVal, 0, 0);
	int rows = 64; //default height size
	int cols = hist.rows; //get the width size from the histogram
	Mat histImg = Mat::zeros(rows*scaleX, cols*scaleY, CV_8UC3);
	//for each bin
	for(int i=0;i<cols-1;i++) {
		float histValue = hist.at<float>(i,0);
		float nextValue = hist.at<float>(i+1,0);
		Point pt1 = Point(i*scaleX, rows*scaleY);
		Point pt2 = Point(i*scaleX+scaleX, rows*scaleY);
		Point pt3 = Point(i*scaleX+scaleX, (rows-nextValue*rows/maxVal)*scaleY);
		Point pt4 = Point(i*scaleX, (rows-nextValue*rows/maxVal)*scaleY);

		int numPts = 5;
		Point pts[] = {pt1, pt2, pt3, pt4, pt1};

		fillConvexPoly(histImg, pts, numPts, Scalar(255,255,255));
	}
	return histImg;
}

void showHistograms(const Mat_<Vec3b> &image, const Mat_<float> &mask, int nbBins) {
	vector<Mat> channels;
	Mat newMask = Mat_<uchar>(mask);

	split(image, channels);

	for (int i = 0; i < channels.size(); i++) {
		Mat histogram;
		int channelInd[1] = {0};
		int histSize[] = {nbBins};
		float hrange[] = {0, 256};
		const float *ranges[] = {hrange};

		calcHist(&channels[i], 1, channelInd, newMask, histogram, 1, histSize, ranges);
		Mat histogramDrawing = imHist(histogram);

		stringstream ss;

		ss<<"channel "<<i;

		imshow(ss.str(), histogramDrawing);
	}
}