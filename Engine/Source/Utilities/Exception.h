#pragma once
#include "Macro.h"

#define THROW_EXCEPTION(_msg) throw Krampus::Exception(_msg, DEBUG_INFO);

namespace Krampus
{

    class Exception : public std::exception
    {
        using Error = std::string;
        Error message;

    public:
        Exception(const Error& _error, const Error& _debugInfo = "");

    private:
        NO_DISCARD virtual const char* what() const noexcept override;
    public:
        NO_DISCARD const char* What() const;
    };

}


/////////////////////////////////////////////////////////
// 
//  try
//  {
//      TROW_EXCEPTION("msg");
//  }
//  catch (const engine::Exception& _exception)
//  {
//      std::cerr << _exception.What() << std::endl;
//  }
// 
/////////////////////////////////////////////////////////