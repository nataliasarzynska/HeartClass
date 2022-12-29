#ifndef DADM_HEART_CLASS_H
#define DADM_HEART_CLASS_H

//--------------------------------------------------------------------------------
/* Includes */

#include <complex>
#include <iostream>
#include <valarray>
#include <vector>
#include <memory>
#include <numeric>

// #include "../Waves/waves.h"
// #include "dbscan.h"

//--------------------------------------------------------------------------------
/* Public types */

/*
 * @brief           Possible types of detected activation
 */
enum type{
    VENTRICULAR = 0,        //< Ventricular activation
    SUPRAVENTRICULAR,       //< Supraventricular activation
};

/*
 * @brief           Struct for storing index and type of classification
 */
typedef struct activations{
    float Ridx;             //< index of R peak associated with activation
    enum type actType;      //< activation type, see enum type
}activations_t;

/*
 * @brief           Struct for storing index and type of classification
 */
typedef struct clusters{
    float Ridx;             //< index of R peak associated with activation
    uint8_t ClusterID;      //< ID of classified cluster
}clusters_t;

/*
 * @brief           Main class of the module.
 * @note            After calling the constructor you can get pointers to vectors of
 *                  classifications with appropriate getters.
 */


class heart_class {
private:
/*
 * @brief           Function for checking if there is AV dissociation in the current part of signal.
 * @note            AV dissociation means that the relation between P-waves and QRS complexes is completely lost.
 *                  P-wave fluctuates back and forth across QRS complexes.
 * @param[in]       waves - pointer to waves of signal
 * @param[in]       currR - current R peak diagnosed
 * @return          classification of activation.
 */
    enum type CheckAVDissociation(std::shared_ptr<WavesFeatures> waves, int currR);

/*
 * @brief           Function for classifying activations.
 * @param[in]       rPeaks - pointer to vector of detected R peaks
 * @param[in]       waves - pointer to waves of signal
 * @param[in]       fs - sampling frequency
 */
    void Classify_Type(std::shared_ptr<std::vector<int>> rPeaks, std::shared_ptr<WavesFeatures> waves, int fs);

/*
 * @brief           Function for classifying activations.
 * @param[in]       rPeaks - pointer to vector of detected R peaks
 * @param[in]       waves - pointer to waves of signal
 * @param[in]       fs - sampling frequency
 */
     void Classify_Cluster(std::shared_ptr<std::vector<int>> rPeaks, std::shared_ptr<WavesFeatures> waves, int fs);

/*
 * @brief           Pointer to vector of activations.
 */
    std::vector<activations_t> activations;

/*
 * @brief           Pointer to vector of clusters.
 */
    std::vector<clusters_t> clusters;

public:

/*
 * @brief           Constructor. Calles processing functions.
 * @param[in]       rPeaks - pointer to vector of detected R peaks
 * @param[in]       waves - pointer to waves of signal
 * @param[in]       fs - sampling frequency
 */
    
    
   //!! heart_class(std::shared_ptr<std::vector<int>> rPeaks, std::shared_ptr<WavesFeatures> waves, int fs);

/*
 * @brief           Getter function for activation types.
 * return           pointer to vector of activation types.
 */
    //!! std::shared_ptr<std::vector<activations_t>> heart_classGetActivations();

/*
 * @brief           Getter function for activation types.
 * return           pointer to vector of clusters.
 */
    //!! std::shared_ptr<std::vector<clusters_t>> heart_classGetClusters();
};

#endif //DADM_HEART_CLASS_H