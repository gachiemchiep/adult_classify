Adult content images classifier.

This program is an implement of 
Jau-Ling Shih 's "An adult image identification system employing image retrieval technique"


Project compile:
1. Download into workspace
2. Use eclipse import 
   File-> Import-> Existing project into workspace-> path to downloaded folder
3. After importing, right click project ->Build project


Usage: program option is as follow

main options:
  -h [ --help ]                         Print help message
  -r [ --rm_bg ]                        Remove background 
  -e [ --extract_feature ]              Extract feature (scd+ehd+cd)
  -t [ --execute_testing ]              Execute testing

Remove background options:
  -b [ --bg_img ] arg                   Image path which is used for removing 
                                        background
  --rm_method arg (=all)                Method for removing background 
                                        Available options: RGB; YCrCb; HSV; 
                                        HLS; RGB_norm; HSI;

Extract features options:
  -p [ --img_path ] arg                 Image path which is used for extracting
                                        feature
  --feature_type arg (=all)             Method for extracting feature.
                                        Currently not usable
  --feature_file arg (=File in which feature will be appended to)

Execute testing options:
  -x [ --adult_features_file ] arg      File contains all adult content 
                                        images's feature vectors
  -y [ --non_adult_features_file ] arg  File contains all no_adult content 
                                        images's feature vectors
  --test_method arg                     Method for testing. 
                                        Available options: scd; ehd; cd; all;




Program detail:
See index.html for more detail.  


実験の詳細:
実験は以下のように行った。

1.データをadult、non-adultに分ける

adult、non-adultというfolderを作成  

adult content imagesをadult folderに移動

non_adult content imagesをnon_adult folderに移動

2.特徴抽出

adult folderの全て画像の特徴を抽出して、adult_features.txtに保存する。

non_adult folderの全て画像の特徴を抽出して、non_adult_features.txtに保存する。

一つの画像file1.jpgの特徴を抽出してfeature.txtファイルに特徴を保存するため、以下のコマンドを実行すれば良い

./program --extract_feature --img_path path/file1.jpg --feature_type all
--feature_file path/feature.txt 

3.実験行う

adult folderの全て画像の特徴はadult_features.txtに保存する。

non_adult folderの全て画像の特徴はnon_adult_features.txtに保存する。

5-foldingで実験を行う。データの20%はテスト用、残る80%は学習用である。

実験行うコマンドは以下のようになる。

./program --execute_testing --adult_features_file adult_features.txt
 --non_adult_features_file non_adult_features.txt --test_method all
 
 サンプル結果
 
[gachiemchiep@gachiemchiep Debug]$ ./adult_classify -t -x ../model_org/adult_feature.txt -y ../model_org/non_adult_feature.txt --test_method all
../model_org/adult_feature.txt ../model_org/non_adult_feature.txt
../model_org/adult_feature.txt
../model_org/non_adult_feature.txt
adult_count:897
non_adult_count:509 
adult_test_count:179
non_adult_test_count:101 
adult_learn_count:718
non_adult_learn_count:408 
Method is ALL
Nearest Neighbord: 112
Classification's accuracy:
Adult:          true_count:121 false_count:58 Per:67%
Non_Adult:      true_count:90 false_count:11 Per:89%
Testing finish!!! 
 