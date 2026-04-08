#ifndef LINEARSCAN_HPP
#define LINEARSCAN_HPP

// don't include other headfiles
#include <string>
#include <vector>
#include <set>

class Location {
public:
    // return a string that represents the location
    virtual std::string show() const = 0;
    virtual int getId() const = 0;
};

class Register : public Location {
private:
    int regId;
public:
    Register(int regId) : regId(regId) {}
    virtual std::string show() const {
        return "reg" + std::to_string(regId);
    }
    virtual int getId() const {
        return regId;
    }
};

class StackSlot : public Location {
public:
    StackSlot() {}
    virtual std::string show() const {
        return "stack";
    }
    virtual int getId() const {
        return -1;
    }
};

struct LiveInterval {
    int startpoint;
    int endpoint;
    Location* location = nullptr;
};

class LinearScanRegisterAllocator {
private:
    int regNum;
    std::vector<int> freeRegs;

    struct CompareEndpoint {
        bool operator()(const LiveInterval* a, const LiveInterval* b) const {
            if (a->endpoint != b->endpoint)
                return a->endpoint < b->endpoint;
            return a < b;
        }
    };
    std::set<LiveInterval*, CompareEndpoint> active;

    void expireOldIntervals(LiveInterval& i) {
        auto it = active.begin();
        while (it != active.end()) {
            LiveInterval* j = *it;
            if (j->endpoint >= i.startpoint) {
                break;
            }
            freeRegs.push_back(j->location->getId());
            it = active.erase(it);
        }
    }

    void spillAtInterval(LiveInterval& i) {
        auto last_it = active.end();
        --last_it;
        LiveInterval* spill = *last_it;

        if (spill->endpoint > i.endpoint) {
            i.location = spill->location;
            spill->location = new StackSlot();
            active.erase(last_it);
            active.insert(&i);
        } else {
            i.location = new StackSlot();
        }
    }

public:
    LinearScanRegisterAllocator(int regNum) : regNum(regNum) {
        for (int i = regNum - 1; i >= 0; --i) {
            freeRegs.push_back(i);
        }
    }

    void linearScanRegisterAllocate(std::vector<LiveInterval>& intervalList) {
        for (auto& i : intervalList) {
            expireOldIntervals(i);
            if (active.size() == regNum) {
                spillAtInterval(i);
            } else {
                int regId = freeRegs.back();
                freeRegs.pop_back();
                i.location = new Register(regId);
                active.insert(&i);
            }
        }
    }
};

#endif
