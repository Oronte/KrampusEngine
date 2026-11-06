#pragma once
#include "Utilities/Macro.h"

#define THROW_EXCEPTION(_msg) throw Krampus::Exception(_msg, DEBUG_INFO);

namespace Krampus
{

    class Exception : public std::exception
    {
        String message;

    public:
        Exception(const String& _error, const String& _debugInfo = "");

    private:
        [[nodiscard]] const char* what() const noexcept override;
    public:
        [[nodiscard]] const char* What() const;
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