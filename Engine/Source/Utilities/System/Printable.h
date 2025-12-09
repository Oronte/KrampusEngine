#pragma once
#include "Utilities/Macro.h"

namespace Krampus
{

	struct IPrintable
	{
		virtual std::string ToString() const = 0;
		friend std::ostream& operator << (std::ostream& _os, const IPrintable& _object)
		{
			_os << _object.ToString();
			return _os;
		}
	};

}