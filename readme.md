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


References