Adult content images classifier.

process is as follow.

1. skin detection
Finding ROI contains skin, remove background
2. extract feature from ROI
CSIFT, shape,...
3. make BOF dictionary
4. Learning images is represented as BOF vectors
5. Input image is represented as BOF vector
6. Count x images which are nearest to input image
Using: Euclid distance, etc

For testing classification's accuracy

extract feature for all image
load feature as a featurematrix
Doing 5-fold testing by
	20% of feature-matrix is testing
	80% of feature-matrix is learning
	after 5 fold, take average 

References

