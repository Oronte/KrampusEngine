#pragma once
#include "Utilities/Macro.h"

namespace Krampus
{

	/// <summary>
	/// Interface used to make objects printable to output streams.
	/// </summary>
	struct IPrintable
	{
		// Convert the object to a string representation.
		virtual std::string ToString() const = 0;
		friend std::ostream& operator << (std::ostream& _os, const IPrintable& _object)
		{
			_os << _object.ToString();
			return _os;
		}
		operator std::string()
		{
			return ToString();
		}
	};

}

/////////////////////////////////////////////////
// 
// class PrintableClass : public IPrintable
// {
//     virtual std::string ToString() const override
//	   {
//         return "PrintableClass representation";
//     }
// }
// 
/////////////////////////////////////////////////