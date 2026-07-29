/*
 * Copyright (C) 2019  Stefan Fabian
 *
 * This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HECTOR_RVIZ_OVERLAY_GL_HELPERS_H
#define HECTOR_RVIZ_OVERLAY_GL_HELPERS_H

#include <GL/gl.h>
#include <iostream>

namespace hector_rviz_overlay
{
inline void _checkError( const char *call, const char *filename, int line )
{
  GLenum err;
  while ( ( err = glGetError() ) != GL_NO_ERROR ) {
    std::cerr << filename << ":" << line << " - " << call << ": GL_ERROR: " << err << std::endl;
  }
}

inline void _clearErrors()
{
  GLenum err;
  while ( ( err = glGetError() ) != GL_NO_ERROR );
}
} // namespace hector_rviz_overlay

#ifdef DEBUG_GL_CALLS
#define HECTOR_RVIZ_OVERLAY_DEBUG_CLEAR_ERRORS() _clearErrors();
#define HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( _call )                                          \
  _call;                                                                                           \
  _checkError( #_call, __FILE__, __LINE__ )
#else
#define HECTOR_RVIZ_OVERLAY_DEBUG_CLEAR_ERRORS()                                                   \
  do {                                                                                             \
  } while ( false );
#define HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( _call ) _call
#endif

#endif // HECTOR_RVIZ_OVERLAY_GL_HELPERS_H
