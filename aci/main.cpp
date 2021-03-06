#include "main.h"

#define DEBUG true
#define STATFOLDER "../stats/"
#define COLOR_SIGMA 25
#define CENTERS_SIGMA 1
#define AREA_SIGMA 250
#define NB_EIGENVECTORS 7

using namespace std;

void matchingImages(const vector<std::tuple<int,int,double> > &matching, DisjointSetForest &seg1, DisjointSetForest &seg2, const Mat_<Vec3f> &image1, const Mat_<Vec3f> &image2, Mat_<Vec3b> &regionImage1, Mat_<Vec3b> &regionImage2) {
	vector<Vec3b> colors1, colors2;
	colors1.reserve(seg1.getNumberOfComponents());
	colors2.reserve(seg2.getNumberOfComponents());

	for (int i = 0; i < seg1.getNumberOfComponents(); i++) {
		colors1.push_back(Vec3b(0,0,0));
	}

	for (int i = 0; i < seg2.getNumberOfComponents(); i++) {
		colors2.push_back(Vec3b(0,0,0));
	}

	for (int i = 0; i < (int)matching.size(); i++) {
		Vec3b randColor(rand()%255, rand()%255, rand()%255);

		colors1[get<0>(matching[i])] = randColor;
		colors2[get<1>(matching[i])] = randColor;
	}

	regionImage1 = seg1.toRegionImage(image1, colors1);
	regionImage2 = seg2.toRegionImage(image2, colors2);
}

int main(int argc, char** argv) {
	char *charaNames[] = {"rufy", "ray", "miku", "majin", "lupin", "kouji", "jigen", "conan", "chirno", "char", "asuka", "amuro", NULL};
	vector<std::tuple<Mat_<Vec3b>, Mat_<float> > > dataset;
	Mat_<int> classes;
	cout<<"loading dataset..."<<endl;
	loadDataSet("../test/dataset/", charaNames, 15, dataset, classes);

	cout<<"preprocessing"<<endl;
	vector<std::tuple<Mat_<Vec3f>, Mat_<float> > > processedDataset;
	processedDataset.reserve(dataset.size());

	for (int i = 0; i < dataset.size(); i++) {
		std::tuple<Mat_<Vec3f>, Mat_<float> > processedSample;

		preProcessing(get<0>(dataset[i]), get<1>(dataset[i]), get<0>(processedSample), get<1>(processedSample));

		processedDataset.push_back(processedSample);
	}

	cout<<"segmentation"<<endl;
	vector<DisjointSetForest> segmentations;
	segmentations.reserve(processedDataset.size());

	for (int i = 0; i < (int)processedDataset.size(); i++) {
		DisjointSetForest segmentation;

		segment(get<0>(processedDataset[i]), get<1>(processedDataset[i]), segmentation);
		segmentations.push_back(segmentation);
	}

	cout<<"classification"<<endl;

	double maxClassLabel;

	minMaxLoc(classes, NULL, &maxClassLabel);

	float rate = 0;
	MatrixXi confusion = MatrixXi::Zero(maxClassLabel + 1, maxClassLabel + 1);
	vector<pair<int,int> > misclassifications;

	for (int i = 0; i < (int)processedDataset.size(); i++) {
		MatchingSegmentClassifier classifier(true);
		cout<<"building training set"<<endl;
		typedef std::tuple<DisjointSetForest, Mat_<Vec3f>, Mat_<float>, int > TrainingSample;

		vector<TrainingSample> trainingSet;
		trainingSet.reserve(processedDataset.size() - 1);

		for (int j = 0; j < (int)processedDataset.size(); j++) {
			if (i != j) {
				trainingSet.push_back(TrainingSample(segmentations[j], get<0>(processedDataset[j]), get<1>(processedDataset[j]), classes(j,0)));
			}
		}

		cout<<"training"<<endl;
		classifier.train(trainingSet);
		cout<<"predicting"<<endl;
		int nearest;
		vector<std::tuple<int,int,double> > bestMatching;
		int actual = classifier.predict(segmentations[i], get<0>(processedDataset[i]), get<1>(processedDataset[i]), &nearest, &bestMatching);

		nearest = nearest < i ? nearest : nearest + 1;

		cout<<"displaying matching"<<endl;
		Mat_<Vec3b> match1, match2;

		matchingImages(bestMatching, segmentations[i], segmentations[nearest], get<0>(processedDataset[i]), get<0>(processedDataset[nearest]), match1, match2);

		waitKey(0);

		cout<<"predicted sample "<<i<<" in class "<<actual<<", expected "<<classes(i,0)<<endl;

		waitKey(0);

		if (actual == classes(i,0)) {
			rate++;
		} else {
			misclassifications.push_back(pair<int,int>(i, nearest));
		}

		confusion(classes(i,0), actual)++;
	}

	rate = rate / (float)processedDataset.size();

	cout<<"recognition rate "<<rate<<endl;
	cout<<"confusion matrix"<<endl<<confusion<<endl;
	cout<<"displaying misclassified samples and nearest neighbor"<<endl;

	for (vector<pair<int,int> >::iterator it = misclassifications.begin(); it != misclassifications.end(); it++) {
		imshow("misclassified", get<0>(dataset[(*it).first]));
		imshow("nearest", get<0>(dataset[(*it).second]));
		waitKey(0);
	}

	return 0;
}
