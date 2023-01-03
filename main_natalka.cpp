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


// ---------- data loader ----------

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

/* Threshold interval based on Brugada algorithm */
#define R_S_INTERVAL            60

/* Number of intervals used for AV dissociation analysis */
#define NUM_OF_INTERVALS        10

/* Standard deviation threshold */
#define STD_TH                  10

/* Threshold value for detecting correct QRSonset/end waves (in miliseconds) */
#define QRS_INTERVAL            80

int SamplesToMiliseconds(int value, int fs)
{
    return value*1000/fs;
}

enum type CheckAVDissociation(std::vector<int> rPeaks, std::vector<int> P, int currR) {

    std::vector<int> intervals;
    std::vector<int> QRSonset = {loadQRSonset()};

    /* Measure last P-QRSonset intervals */
    auto itQRS = std::upper_bound(QRSonset.begin(), QRSonset.end(), currR);
    auto itP = std::upper_bound(P.begin(), P.end(), currR);
    int currInterval;

    for(uint8_t i = 0; i < NUM_OF_INTERVALS; i++)
    {
        /* Check if reached end of signal (inna choroba by default) */
        if(itQRS == QRSonset.end() || itP == P.end())
            return DIFF_DISEASE;

        currInterval = *itQRS++ - *itP++;
        intervals.push_back(currInterval);

        int currInterval_ms = SamplesToMiliseconds(currInterval, 360);

    }

    /* Calculate standard deviation */
    int sum = std::accumulate(intervals.begin(), intervals.end(), 0.0);
    int mean = sum/intervals.size();
    int stdDev = 0;
    int temp;
    for(const auto &el : intervals)
    {
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

void Classify_Type(std::vector<int> rPeaks, std::vector<int> P, std::vector<int> QRSend, std::vector<int> QRSonset, int fs) 
    {
    activations_t nextClassification;

    auto S = QRSend;
    auto Q = QRSonset; 

    for(const auto &r : rPeaks)
    {
        // Szukamy zalamka Q
        auto it_q = std::lower_bound(Q.begin(), Q.end(), r); 
        it_q -= 1;

        // Szukamy zalamka S
        auto it_s = std::upper_bound(S.begin(), S.end(), r); 
        
        if(it_s != S.end())
        {
            int intervalRS = SamplesToMiliseconds(*it_s - r , fs);
            int intervalQRS = SamplesToMiliseconds(*it_s - *it_q, fs);

            int QRS = *it_s - *it_q;
            int QRSms = SamplesToMiliseconds(QRS, fs);

            if(QRSms < 180 && QRSms > 50) { // sprawdzenie czy to artefakt czy QRS
                if(QRSms >= 120) {
                    // szeroki - to znaczy ze cos jest nie tak!!!!!!
                    // VR albo inna choroba
                    if(intervalRS < R_S_INTERVAL )
                    {
                        // Sprawdzamy czy jest dysocjacja
                        nextClassification.actType = CheckAVDissociation(rPeaks, P, r);            

                        if(nextClassification.actType == 1)
                        {
                            cout << "VENTRICULAR\n" << endl; // wykrywamy dysocjacje -> TU MA BYC VENTRICULAR
                        }
                        else 
                        {
                            cout << "DIFFERENT DISEASE\n" << endl; // nie ma dysocjacji -> chyba masz inna chorobe ziomus
                        }
                    }
                    else
                    {
                        // jest ventricular 
                        nextClassification.actType = VENTRICULAR;
                        cout << "If greater than 60 it favours VERTICULAR" << endl;
                    }
                }
                else {
                    // waski
                    // SVR
                    cout << "SVR" << endl;
                }
                
            } else { // artefakt
                cout << "Artefakt" << endl;
                // return ARTEFAKT

            }

            nextClassification.Ridx = r;
           
            cout << "----------------------------------- \n" << endl;
        }
    }
}


int main(){

    // load data
    vector <float> filteredSignal {loadSignal()};
    vector <int> rPeaks {loadRpeaks()};
    vector<int> rPeaksLocs(rPeaks.begin(), rPeaks.end());
    vector <int> QRSend {loadQRSend()};
    vector <int> QRSonset {loadQRSonset()};
    vector <int> P {loadP()};

    // check if data was loaded correctly

    cout << "count of samples in signal = " << filteredSignal.size() << endl;
    cout << "count of R peaks = " << rPeaks.size() << endl;
    cout << "count of QRSend = " << QRSend.size() << endl;


    Classify_Type(rPeaks,P,  QRSend, QRSonset,  360); 

  






}

