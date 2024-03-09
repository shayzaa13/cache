#include <iostream>
#include <fstream>
#include <string>
#include <utility> // For tuple
#include <list>
#include <vector>

using namespace std;

struct Tuple {
    int first;
    int second;

    Tuple(int f, int s) : first(f), second(s) {}
};

vector<Tuple> tableData;
vector<Tuple> tlbAccesses;

int vpn_;
int pfn_;
int page_offset_;
int tlbSize;
int decimalVPN;
int decimalPFN;
int size;

string skipLine;

int missescount = 0;
int hitscount = 0;

int convert(int a, int n, int b) {
    return (a << n) + b;
}

class TLB {
public:
    list<Tuple> tlbEntries;

    Tuple VPNLookUp(const int vpn) {
        for (auto it = tlbEntries.begin(); it != tlbEntries.end(); ++it) {
            if (it->first == vpn) {
                Tuple result = *it;
                tlbEntries.erase(it);
                tlbEntries.push_front(result);
                return result;
            }
        }
        return Tuple(-1, -1);
    }

    void AddEntry(int vpn, int pfn) {
        if (tlbEntries.size() >= tlbSize) {
            tlbEntries.pop_back();
            tlbEntries.push_front(Tuple(vpn, pfn));
        } else {
            tlbEntries.push_front(Tuple(vpn, pfn));
        }
    }

    Tuple TLBAccess(int vpn) {
        Tuple result = VPNLookUp(vpn);

        if (result.first  ==-1) {
            for (const auto &entry : tableData) {
                if (entry.first == vpn) {
                    AddEntry(vpn, entry.second);
                    return Tuple(entry.second, 0);
                }
            }

            return Tuple(-1, 0);
        } else {
            return Tuple(result.second, 1);
        }
    }

    TLB(const string &filename) {
        ifstream file(filename);

        file >> page_offset_;

        int VA;
        while (file >> VA) {
            int PageOffset = VA & ((1 << page_offset_) - 1);
            int VPN = VA >> page_offset_;

            tlbAccesses.push_back(Tuple(VPN, PageOffset));
        }

        file.close();
    }
};

int main(int argc, char *argv[]) {
    tlbSize = stoi(argv[4]);
    ifstream file(argv[1]);

    file >> vpn_ >> pfn_;
    file >> skipLine >> skipLine;

    while (file >> decimalVPN >> decimalPFN) {
        tableData.push_back(Tuple(decimalVPN, decimalPFN));
    }

    file.close();

    TLB tlb(argv[2]);

    vector<Tuple> accesses;

    for (const auto &access : tlbAccesses) {
        Tuple accessOutput = tlb.TLBAccess(access.first);
        accesses.push_back(accessOutput);
    }
    string ashbdas = argv[1];
    ofstream outwritefile("22116090_" + ashbdas + "_" + argv[2] + "_LRU_" + to_string(tlbSize));

    outwritefile << "TOTAL_ACCESSES = " << accesses.size() << endl;

    for (const auto &output : accesses) {
        if (output.second) {
            hitscount++;
        } else {
            missescount++;
        }
    }
    outwritefile << "TOTAL_MISSES = " << missescount << endl;
    outwritefile << "TOTAL_HITS = " << hitscount << endl;
    string policy = argv[3];

    for (int i = 0; i < accesses.size(); i++) {
        int value = convert(accesses[i].first, page_offset_, tlbAccesses[i].second);
        if (accesses[i].first << page_offset_ < 0) {
            cout << accesses[i].first << page_offset_ << endl;
        }
        if (accesses[i].second) {
            outwritefile << value << " HIT" << endl;
        } else {
            outwritefile << value << " MISS" << endl;
        }
    }

    outwritefile.close();

    return 0;
}