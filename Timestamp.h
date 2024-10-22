#pragma once
#include <iostream>


namespace WYXB
{
    
class Timestamp
{
private:
    int64_t m_microSecondsSinceEpoch_;
public:
    Timestamp();
    explicit Timestamp(int64_t m_microSecondsSinceEpoch);
    static Timestamp now();
    std::string toString() const;

};





}