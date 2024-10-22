#include "Timestamp.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace WYXB
{
   
Timestamp::Timestamp(): m_microSecondsSinceEpoch_(0)
{

    
}
Timestamp::Timestamp(int64_t m_microSecondsSinceEpoch) : m_microSecondsSinceEpoch_(m_microSecondsSinceEpoch)
{
    
}
Timestamp Timestamp::now()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    return Timestamp(micros);

}
std::string Timestamp::toString() const
{
    auto time_point = std::chrono::system_clock::time_point(
            std::chrono::microseconds(m_microSecondsSinceEpoch_)
        );

    // 将 time_point 转换为 time_t
    std::time_t t = std::chrono::system_clock::to_time_t(time_point);

    // 格式化时间字符串
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%d %X");

    return ss.str();
}


}


#ifdef TIMESTAMP_TEST
int main()
{
    WYXB::Timestamp t = WYXB::Timestamp::now();
    std::cout << t.toString() << std::endl;
}
#endif