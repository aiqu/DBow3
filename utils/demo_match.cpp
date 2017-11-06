/**
 * Date:  2016
 * Author: Rafael Muñoz Salinas
 * Description: demo application of DBoW3
 * License: see the LICENSE.txt file
 */

#include <iostream>
#include <vector>

// DBoW3
#include "DBoW3.h"

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#ifdef USE_CONTRIB
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <opencv2/xfeatures2d.hpp>
#endif
#include "DescManip.h"

using namespace DBoW3;
using namespace std;


//command line parser
class CmdLineParser{int argc; char **argv; public: CmdLineParser(int _argc,char **_argv):argc(_argc),argv(_argv){}  bool operator[] ( string param ) {int idx=-1;  for ( int i=0; i<argc && idx==-1; i++ ) if ( string ( argv[i] ) ==param ) idx=i;    return ( idx!=-1 ) ;    } string operator()(string param,string defvalue="-1"){int idx=-1;    for ( int i=0; i<argc && idx==-1; i++ ) if ( string ( argv[i] ) ==param ) idx=i; if ( idx==-1 ) return defvalue;   else  return ( argv[  idx+1] ); }};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// extended surf gives 128-dimensional vectors
const bool EXTENDED_SURF = false;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void wait()
{
    cout << endl << "Press enter to continue" << endl;
    getchar();
}


vector<string> readImagePaths(int argc,char **argv,int start){
    vector<string> paths;
    for(int i=start;i<argc;i++)    paths.push_back(argv[i]);
        return paths;
}

vector< cv::Mat  >  loadFeatures( std::vector<string> path_to_images,string descriptor="") throw (std::exception){
    //select detector
    cv::Ptr<cv::Feature2D> fdetector;
    if (descriptor=="orb")        fdetector=cv::ORB::create();
    else if (descriptor=="brisk") fdetector=cv::BRISK::create();
#ifdef OPENCV_VERSION_3
    else if (descriptor=="akaze") fdetector=cv::AKAZE::create();
#endif
#ifdef USE_CONTRIB
    else if(descriptor=="surf" )  fdetector=cv::xfeatures2d::SURF::create(400, 4, 2, EXTENDED_SURF);
#endif

    else throw std::runtime_error("Invalid descriptor");
    assert(!descriptor.empty());
    vector<cv::Mat>    features;


    cout << "Extracting   features..." << endl;
    for(size_t i = 0; i < path_to_images.size(); ++i)
    {
        vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        //cout<<"reading image: "<<path_to_images[i]<<endl;
        cv::Mat image = cv::imread(path_to_images[i], 0);
        if(image.empty())throw std::runtime_error("Could not open image"+path_to_images[i]);
        //cout<<"extracting features"<<endl;
        fdetector->detectAndCompute(image, cv::Mat(), keypoints, descriptors);
        features.push_back(descriptors);
        //cout<<"done detecting features"<<endl;
    }
    return features;
}

// ----------------------------------------------------------------------------

void testDatabase(const  vector<cv::Mat > &features)
{
    cout << "Creating a small database..." << endl;

    // load the vocabulary from disk
    Vocabulary voc("orbvoc.dbow3");

    Database db(voc, false, 0); // false = do not use direct index
    // (so ignore the last param)
    // The direct index is useful if we want to retrieve the features that
    // belong to some vocabulary node.
    // db creates a copy of the vocabulary, we may get rid of "voc" now

    // add images to the database
	cout << features.size() << endl;
	for(auto f : features)
		db.add(f);

    cout << "... done!" << endl;

    cout << "Database information: " << endl << db << endl;

    // and query the database
    cout << "Querying the database: " << endl;

    QueryResults ret;
    for(size_t i = 0; i < features.size(); i++)
    {
        db.query(features[i], ret, 2);

        // ret[0] is always the same image in this case, because we added it to the
        // database. ret[1] is the second best match.

        cout << "Searching for Image " << i << ". " << ret << endl;
    }

    cout << endl;
}

vector<cv::Mat> readFeaturesFromFile(string filename){
vector<cv::Mat> features;
    //test it is not created
    std::ifstream ifile(filename);
    if (!ifile.is_open()){cerr<<"could not open input file"<<endl;exit(0);}
    uint32_t size;
    ifile.read((char*)&size,sizeof(size));
    features.resize(size);
    for(size_t i=0;i<size;i++){

        uint32_t cols,rows,type;
        ifile.read( (char*)&cols,sizeof(cols));
        ifile.read( (char*)&rows,sizeof(rows));
        ifile.read( (char*)&type,sizeof(type));
        features[i].create(rows,cols,type);
        ifile.read( (char*)features[i].ptr<uchar>(0),features[i].total()*features[i].elemSize());
    }
    return features;
}

// ----------------------------------------------------------------------------

int main(int argc,char **argv)
{

    try{
        CmdLineParser cml(argc,argv);
        if (cml["-h"] || argc<=2){
            cerr<<"Usage:  descriptor_name featurefile \n\t descriptors:brisk,surf,orb ,akaze(only if using opencv 3)"<<endl;
             return -1;
        }

		string descriptor=argv[1];

        //auto images=readImagePaths(argc,argv,2);
        //vector< cv::Mat   >   features= loadFeatures(images,descriptor);
        auto features=readFeaturesFromFile(argv[2]);
        testDatabase(features);

    }catch(std::exception &ex){
        cerr<<ex.what()<<endl;
    }

    return 0;
}
