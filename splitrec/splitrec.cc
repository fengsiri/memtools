#include "mminfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>

int main( int argc, char* argv[] )
{
	for( int i = 1; i < argc; ++i )
	{
		int count = 0;
		std::filebuf buf;
		buf.open( argv[i], std::ios_base::in | std::ios_base::binary );
		if( buf.is_open() )
		{
			MemMon::MemoryMap mm;
			std::streambuf* bufptr = &buf;
			mm.Read( bufptr );

			while( !std::filebuf::traits_type::eq_int_type( buf.sgetc(), std::filebuf::traits_type::eof() ) )
			{
				MemMon::MemoryDiff md;
				md.Read( bufptr );
				md.Apply( mm );

				std::ostringstream fnamemaker;
				fnamemaker << argv[i] << "_SPLIT_" << std::setw(3) << std::setfill('0') << ++count;

				std::filebuf out;
				out.open( fnamemaker.str().c_str(), std::ios_base::out | std::ios_base::binary );

				if( out.is_open() )
				{
					std::streambuf* outptr = &out;
					mm.Write( outptr );
				}
			}
		}
	}
	return 0;
}
