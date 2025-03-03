#ifndef LivEInterval_H
#define LivEInterval_H
#include "../machine_pass.h"
#include <assert.h>
#include <queue>
class LiveInterval {
private:
    Register reg;
    // 当begin和end不同时, 活跃区间为[begin,end), 即左闭右开
    // 当begin和end相同时, 表示[begin,end], 即一个单点 (这么做的原因是方便活跃区间计算)
    // 注意特殊判断begin和end相同时的情况
    struct LiveSegment {
        int begin;
        int end;
        bool inside(int pos) const {
            if (begin == end) return begin == pos;
            return begin <= pos && pos < end; 
        }
        bool operator&(const struct LiveSegment &that) const {
            return this->inside(that.begin) || this->inside(that.end - 1 > that.begin ? that.end - 1 : that.begin) ||
                   that.inside(this->begin) || that.inside(this->end - 1 > this->begin ? this->end - 1 : this->begin);
        }
        bool operator==(const struct LiveSegment &that) const {
            return this->begin == that.begin && this->end == that.end;
        }
    };
    //LiveInterval包含有许多livesegment
    std::list<LiveSegment> segments{};
    int reference_count;

public:
    // 检测两个活跃区间是否重叠
    // 保证两个活跃区间各个段各自都是不降序（升序）排列的
    //Reference: https://github.com/yuhuifishash/SysY
    bool operator&(const LiveInterval &that) const {
        //TODO("& operator in LiveInterval");
        if (segments.empty() || that.segments.empty())
        {
            return false;
        }
        else
        {
            auto it = segments.begin();
            auto jt = that.segments.begin();
            while (1) {
                if (*it & *jt) {
                    return true;//如果有重合的直接返回true，其他任何情况都false
                }
                //哪个比较靠前就把哪个往后遍历找下一个段->这样就要求必须是顺序的
                if (it->end <= jt->begin) {
                    ++it;
                    if (it == segments.end()) {
                        return false;
                    }
                } else if (jt->end <= it->begin) {
                    ++jt;
                    if (jt == that.segments.end()) {
                        return false;
                    }
                } else {
                    ERROR("LiveInterval::operator&: Error");
                }
            }
            }
            return false;
    }

    // 更新引用计数
    void IncreaseReferenceCount(int count) { reference_count += count; }
    int getReferenceCount() { return reference_count; }
    // 返回活跃区间长度
    int getIntervalLen() {
        int ret = 0;
        for (auto seg : segments) {
            ret += (seg.end - seg.begin + 1);
        }
        return ret;
    }
    Register getReg() { return reg; }
    LiveInterval() : reference_count(0) {}    // Temp
    LiveInterval(Register reg) : reg(reg), reference_count(0) {}

    void PushFront(int begin, int end) { segments.push_front({begin = begin, end = end}); }
    void SetMostBegin(int begin) { segments.begin()->begin = begin; }

    // 可以直接 for(auto segment : liveinterval)
    decltype(segments.begin()) begin() { return segments.begin(); }
    decltype(segments.end()) end() { return segments.end(); }
};

class Liveness {
private:
    MachineFunction *current_func;
    // 更新所有块DEF和USE集合
    void UpdateDefUse();
    // Key: Block_Number
    // 存储活跃变量分析的结果
    std::map<int, std::set<Register>> IN{}, OUT{}, DEF{}, USE{};
    // DEF[B]: 在基本块B中定义，并且定义前在B中没有被使用的变量集合
    // USE[B]: 在基本块B中使用，并且使用前在B中没有被定义的变量集合
    // IN[B]: 进入基本块 B 时的活跃变量集合，即这些变量的值可能在 B 中被使用，
    //        它们的值是从 B 的前驱基本块中传递过来的，或者是在 B 中被使用且在 B 之前未被定义的变量
    // OUT[B]: 离开基本块 B 时的活跃变量集合，即这些变量的值可能在 B 的后继基本块中被使用，
    //         它们的值是在 B 中被定义且可能在后继基本块中继续使用，或者是从 B 的前驱基本块传递过来的在 B 中未被重新定义的变量


public:
    // 对所有块进行活跃变量分析并保存结果
    void Execute();
    Liveness(MachineFunction *mfun, bool calculate = true) : current_func(mfun) {
        if (calculate) {
            Execute();
        }
    }
    // 获取基本块的IN/OUT/DEF/USE集合
    std::set<Register> GetIN(int bid) { return IN[bid]; }
    std::set<Register> GetOUT(int bid) { return OUT[bid]; }
    std::set<Register> GetDef(int bid) { return DEF[bid]; }
    std::set<Register> GetUse(int bid) { return USE[bid]; }
};
#endif