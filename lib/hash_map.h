/*
    Copyright 2008 Brain Research Institute, Melbourne, Australia

    Written by J-Donald Tournier, 27/06/08.

    This file is part of MRtrix.

    MRtrix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MRtrix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

    17-03-2009 J-Donald Tournier <d.tournier@brain.org.au>
    * Use the C++ TR1 unordered_map instead of hash_map by default
    * This avoids compilation warnings on newer compilers.
    * The old SGI hash_map can be used by removing the MRTRIX_USE_TR1 macro on the command-line
    * (i.e. remove the '-DUSE_TR1' entry in the cpp_flags line of the relevant sysconf file).

*/

#ifndef __hash_map_h__
#define __hash_map_h__

#define MRTRIX_HASH_MAP_TYPE std::hash_map

#ifdef __clang__
#  include <unordered_map>
#  undef MRTRIX_HASH_MAP_TYPE
#  define MRTRIX_HASH_MAP_TYPE std::unordered_map
#else
#  ifdef MRTRIX_USE_TR1
#    include <tr1/unordered_map>
#    undef MRTRIX_HASH_MAP_TYPE
#    define MRTRIX_HASH_MAP_TYPE std::tr1::unordered_map
#  else
#    ifdef __GNUC__
#      if __GNUC__ < 3
#        include <hash_map.h>
#        undef MRTRIX_HASH_MAP_TYPE
#        define MRTRIX_HASH_MAP_TYPE ::hash_map
#      else
#        include <ext/hash_map>
#        undef MRTRIX_HASH_MAP_TYPE
#        define MRTRIX_HASH_MAP_TYPE ::__gnu_cxx::hash_map
#      endif
#    endif
#  endif
#endif

namespace MR
{
  template <class K, class V> struct UnorderedMap {
    typedef
    MRTRIX_HASH_MAP_TYPE <K,V> Type;
  };
}

#undef MRTRIX_HASH_MAP_TYPE

#endif
