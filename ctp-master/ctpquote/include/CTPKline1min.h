#ifndef CTPKLINE1MIN_H
#define CTPKLINE1MIN_H

#include<string>
#include<ctime>
#include<map>

struct OHLCV
{
    std::string symbol;
    std::time_t dt;
    double      open;
    double      high;
    double      low;
    double      close;
    int         opi;
    int         volume;
};

typedef std::map<std::string, OHLCV> CMapLastKline;

#endif