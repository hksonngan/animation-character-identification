#include "main.h"

#define DEBUG true
#define STATFOLDER "../stats/"
#define COLOR_SIGMA 25
#define CENTERS_SIGMA 1
#define AREA_SIGMA 250
#define NB_EIGENVECTORS 7

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
	loadDataSet("../test/dataset/", charaNames, 5, dataset, classes);

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
		
	MatchingSegmentClassifier classifier(true);

	float bestRate = 0;
	int bestK = 0;

	for (int k = 6; k < 7; k++) {
		cout<<"for K = "<<k<<endl;
		float rate = 0;
		MatrixXi confusion = MatrixXi::Zero(maxClassLabel + 1, maxClassLabel + 1);

		for (int i = 0; i < (int)processedDataset.size(); i++) {
			cout<<"building training set"<<endl;
			typedef std::tuple<DisjointSetForest, Mat_<Vec3f>, Mat_<float>, int > TrainingSample;

			vector<TrainingSample> trainingSet;
			trainingSet.reserve(processedDataset.size() - 1);

			for (int j = 0; j < (int)processedDataset.size(); j++) {
				if (i != j) {
					trainingSet.push_back(TrainingSample(segmentations[j], get<0>(processedDataset[j]), get<1>(processedDataset[j]), classes(j,0)));
				}
			}

			classifier.train(trainingSet);
			int actual = classifier.predict(segmentations[i], get<0>(processedDataset[i]), get<1>(processedDataset[i]), NULL, k);

			cout<<"predicted sample "<<i<<" in class "<<actual<<", expected "<<classes(i,0)<<endl;

			if (actual == classes(i,0)) {
				rate++;
			}

			confusion(classes(i,0), actual)++;
		}

		rate = rate / (float)processedDataset.size();

		if (rate > bestRate) {
			bestRate = rate;
			bestK = k;
		}

		cout<<"recognition rate "<<rate<<endl;
		cout<<"confusion matrix:"<<endl<<confusion<<endl;
	}

	cout<<"best recognition rate "<<bestRate<<" for k = "<<bestK<<endl;

	return 0;
}
