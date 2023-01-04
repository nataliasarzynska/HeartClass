#ifndef DADM_HEART_CLASS_H
#define DADM_HEART_CLASS_H

#include <complex>
#include <iostream>
#include <valarray>
#include <vector>
#include <memory>
#include <numeric>
struct WavesFeatures {
    std::vector<int> QRSonset;
    std::vector<int> QRSend;
    std::vector<int> T;
    std::vector<int> Tend;
    std::vector<int> P;
    std::vector<int> Ponset;
    std::vector<int> Pend;
};

//--------------------------------------------------------------------------------------
/* Public types */

enum type{
    DIFF_DISEASE = 0,        
    VENTRICULAR = 1,
    SUPRAVENTRICULAR = 2,
    ARTIFACT = 3,     
};

typedef struct activations{
    float Ridx;             
    enum type actType;     
}activations_t;

class heart_class {
private:
    enum type CheckAVDissociation(std::shared_ptr<WavesFeatures> waves, int currR);
    void Classify_Type(std::shared_ptr<std::vector<int>> rPeaks, std::shared_ptr<WavesFeatures> waves, int fs);
    std::vector<activations_t> activations;

public:
    heart_class(std::shared_ptr<std::vector<int>> rPeaks, std::shared_ptr<WavesFeatures> waves, int fs);
    std::shared_ptr<std::vector<activations_t>> heart_classGetActivations();
};

#endif //DADM_HEART_CLASS_H