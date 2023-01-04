#include <iostream>
#include <vector>
#include <fstream>
#include "heart_class_m.h"
//#include "waves.h"

using namespace std;

const string signal_path = "100_MLII_filtered.dat";
const string r_peaks_path = "R-peaks.dat";
const string QRSend_path = "QRS-end.txt";
const string QRSonset_path = "QRS-onset.txt";
const string Ponset_path = "P-onset.txt";
const string Pend_path = "P-end.txt";

vector<float> loadSignal() {
    cout << "[INFO] Loading signal..." << endl;

    ifstream fin(signal_path);
    float num;
    vector<float> signal;

    while (fin >> num)
        signal.push_back(num);

    return signal;
}

vector<int> loadRpeaks() {
    cout << "[INFO] Loading r_peaks..." << endl;

    ifstream fin(r_peaks_path);
    float num;
    vector<int> r_peaks;

    while (fin >> num)
        r_peaks.push_back(num);

    return r_peaks;
}

vector<int> loadQRSend() {
    cout << "[INFO] Loading QRSend..." << endl;

    ifstream fin(QRSend_path);
    float num;
    vector<int> QRSend;

    while (fin >> num)
        QRSend.push_back(num);

    return QRSend;
}

vector<int> loadQRSonset() {
    cout << "[INFO] Loading QRSend..." << endl;

    ifstream fin(QRSonset_path);
    float num;
    vector<int> QRSonset;

    while (fin >> num)
        QRSonset.push_back(num);

    return QRSonset;
}

vector<int> loadP() {
    cout << "[INFO] Loading P onset..." << endl;

    ifstream fin(Ponset_path);
    float num;
    vector<int> P;

    while (fin >> num)
        P.push_back(num);

    return P;
}

// ------------------ CONSTANTS -----------------------

// RS length threshold based on Brugada algorithm 
#define R_S_INTERVAL            60

// Number of intervals used for AV dissociation analysis 
#define NUM_OF_INTERVALS        10

// Standard deviation threshold (AV dissociation analysis)
#define STD_TH                  10

int SamplesToMiliseconds(int value, int fs) {
    return value*1000/fs;
}

enum type heart_class::CheckAVDissociation(std::shared_ptr<WavesFeatures> waves, int currR) {
    std::vector<int> intervals;
    auto QRSonset = waves->QRSonset;
    auto P = waves->P;

    auto itQRS = std::upper_bound(QRSonset.begin(), QRSonset.end(), currR);
    auto itP = std::upper_bound(P.begin(), P.end(), currR);
    int currInterval;

    for(uint8_t i = 0; i < NUM_OF_INTERVALS; i++) {
        // Sprawdzamy czy jest juz koniec sygnalu (DIFFERENT DISEASE by default)
        if(itQRS == QRSonset.end() || itP == P.end())
            return DIFF_DISEASE;

        currInterval = *itQRS++ - *itP++;
        intervals.push_back(currInterval);

        int currInterval_ms = SamplesToMiliseconds(currInterval, 360);
    }

    // Odchylenie standardowe
    int sum = std::accumulate(intervals.begin(), intervals.end(), 0.0);
    int mean = sum/intervals.size();
    int stdDev = 0;
    int temp;
    for(const auto &el : intervals) {
        temp = (el-mean)*(el-mean);
        stdDev += temp;
    }
    stdDev /= intervals.size();
    stdDev = std::sqrt(stdDev);
    cout << stdDev << endl;
    cout << "stdDev" << endl;

    if(stdDev < STD_TH)
        return DIFF_DISEASE;
    else
        return VENTRICULAR;
}

void heart_class::Classify_Type(std::shared_ptr<std::vector<int>> rPeaks, std::shared_ptr<WavesFeatures> waves, int fs)  {
    activations_t nextClassification; // do tego zapisujemy diagnoze i wywolujemy to publicznie funkcja getActivations

    auto Q = waves->QRSonset;  // zalamki Q
    auto S = waves->QRSend; // zalamki S

    for(const auto &r : *rPeaks) {
        // Szukamy zalamka Q
        auto it_q = std::lower_bound(Q.begin(), Q.end(), r); 
        it_q -= 1;

        // Szukamy zalamka S
        auto it_s = std::upper_bound(S.begin(), S.end(), r); 
        
        if(it_s != S.end()) {
            int intervalRS = SamplesToMiliseconds(*it_s - r , fs);
            int intervalQRS = SamplesToMiliseconds(*it_s - *it_q, fs);

            int QRS = *it_s - *it_q;
            int QRSms = SamplesToMiliseconds(QRS, fs);

            // sprawdzenie czy to artefakt czy QRS
            if(QRSms < 180 && QRSms > 50) { 
                if(QRSms >= 120) {
                    // szeroki - to znaczy ze cos jest nie tak!!!!!!
                    // VR albo inna choroba
                    if(intervalRS < R_S_INTERVAL ) {
                        // Sprawdzamy czy jest dysocjacja
                        nextClassification.actType = CheckAVDissociation(waves, r);            

                        if(nextClassification.actType == 1)
                        {
                            nextClassification.actType = VENTRICULAR;
                            cout << "VENTRICULAR\n" << endl; // wykrywamy dysocjacje -> jest VENTRICULAR
                        }
                        else 
                        {
                            nextClassification.actType = DIFF_DISEASE;
                            cout << "DIFFERENT DISEASE\n" << endl; // nie ma dysocjacji -> to musi byc inna choroba
                        }
                    }
                    else {
                        // jest ventricular 
                        nextClassification.actType = VENTRICULAR;
                        cout << "If greater than 60 it favours VERTICULAR" << endl;
                    }
                }
                else {
                    // waski
                    // SVR
                    nextClassification.actType = SUPRAVENTRICULAR;
                    cout << "SVR" << endl;
                }
                
            } else { // artefakt
                cout << "Artefakt" << endl;
                nextClassification.actType = ARTIFACT;
            }

           
            this->activations.push_back(nextClassification);
            cout << "----------------------------------- \n" << endl;
        }
    }
}

heart_class::heart_class(std::shared_ptr<std::vector<int>> rPeaks, std::shared_ptr<WavesFeatures> waves, int fs)
{
    Classify_Type(rPeaks, waves, fs);
}

std::shared_ptr<std::vector<activations_t>> heart_class::heart_classGetActivations()
{
    cout<< "?????" << endl;
    return std::make_shared<std::vector<activations_t>>(this->activations);
}

