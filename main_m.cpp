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
const string P_path = "P-onset.txt";

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
    cout << "[INFO] Loading P..." << endl;

    ifstream fin(P_path);
    float num;
    vector<int> P;

    while (fin >> num)
        P.push_back(num);

    return P;
}

// ------------------ CONSTANTS -----------------------

#define SAMPLES_TO_MSECS(_val, _fs) (_val*1000/_fs)

/* Threshold interval based on Brugada algorithm */
#define R_S_INTERVAL            360

/* Number of intervals used for AV dissociation analysis */
#define NUM_OF_INTERVALS        10

/* Standard deviation threshold */
#define STD_TH                  1000

/* Threshold value for detecting correct QRSonset/end waves (in miliseconds) */
#define QRS_INTERVAL            80

/* Minimum points in possible cluster for dbscan algorithm */
#define MINIMUM_POINTS          2

/* Value of epsilon for dbscan algorithm */
#define EPSILON                 2

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
        /* Check if reached end of signal (supraventricular by default) */
        if(itQRS == QRSonset.end() || itP == P.end())
            return SUPRAVENTRICULAR;;

        /* currInterval = *itP++ - *itQRS++; */ // MOJA ZMIANA BO WYCHODZI UJEMNE BO P JEST PIERWSZE
        currInterval = *itQRS++ - *itP++;
        intervals.push_back(currInterval);

        cout << "Current QRS interval samples:" << currInterval << "\n" << endl;  

        int currInterval_ms = SamplesToMiliseconds(currInterval, 360);
        cout << "Current QRS interval ms:" << currInterval_ms << "\n" << endl;  

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

    if(stdDev < STD_TH)
        return SUPRAVENTRICULAR;
    else
        return VENTRICULAR;
}

void Classify_Type(std::vector<int> rPeaks, std::vector<int> P, std::vector<int> QRSend, int fs) 
    {
    activations_t nextClassification;

    auto S = QRSend;

    for(const auto &r : rPeaks)
    {
        /* Search for S wave in current complex */
        auto it = std::upper_bound(S.begin(), S.end(), r);   // r to po kolei podawane kolejne z rPeakow

        if(it != S.end())
        {
            /* Check if S wave detected or in specific range */
            int interval = SamplesToMiliseconds(*it - r , fs);

            cout << "R peak number: "<< r << "\n" << endl;
            cout << "It*: "<< *it << "\n" << endl;
            cout << "Interval time: "<< interval << "\n" << endl;

            if(interval < R_S_INTERVAL ) // SPRAWDZENIE RS - JESLI WIEKSZE NIZ 60 TE KOMOROWE A JAK NIE TO SPRAWDZAMY
            {
                /* Check if AV dissociation is present */
                nextClassification.actType = CheckAVDissociation(rPeaks, P, r);
                cout << "CheckAVDissociation\n" << endl;                

                if(nextClassification.actType == 1)
                {
                    cout << "SUPRAVENTRICULAR detected\n" << endl;
                }
                else{
                    cout << "VENTRICULAR detected\n" << endl;
                }

            }
            else
            {
                /* Ventricular activation detected */
                nextClassification.actType = VENTRICULAR;
                cout << "If greater than 60 it favours VERTICULAR" << endl;
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
    vector <int> P {loadP()};

    // check if data was loaded correctly

    cout << "count of samples in signal = " << filteredSignal.size() << endl;
    cout << "count of R peaks = " << rPeaks.size() << endl;
    cout << "count of QRSend = " << QRSend.size() << endl;


    Classify_Type(rPeaks,P,  QRSend, 60);




}

