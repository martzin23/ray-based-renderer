#include "date_time.h"

std::string getDateTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    struct std::tm local_time;
    localtime_s(&local_time, &now_time);

    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H-%M-%S", &local_time);
    std::string result = buffer;
    return result;
}