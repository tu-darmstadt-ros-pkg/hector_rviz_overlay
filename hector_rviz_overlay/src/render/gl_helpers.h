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
#include <fstream>

namespace hector_rviz_overlay
{
inline void _checkError( const char *call, const char *filename, int line )
{
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR )
  {
    std::cerr << filename << ":" << line << " - " << call << ": GL_ERROR: " << err << std::endl;
  }
}

inline void _clearErrors()
{
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR );
}

inline void dumpTexture( GLuint texture, const char *path )
{
  glBindTexture( GL_TEXTURE_2D, texture );
  GLint width;
  glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
  GLint height;
  glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );
  GLubyte data[width * height * 4];
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
  unsigned int size = 138 + width * height * 4;
  unsigned char bmpData[] = // All values are little-endian
    {
      0x42, 0x4D,             // Signature 'BM'
      static_cast<unsigned char>(size & 255u), static_cast<unsigned char>((size >> 8u) & 255u),
      static_cast<unsigned char>((size >> 16u) & 255u), 0x00, // Size: 138 bytes + image bytes
      0x00, 0x00,             // Unused
      0x00, 0x00,             // Unused
      0x8a, 0x00, 0x00, 0x00, // Offset to image data

      0x7c, 0x00, 0x00, 0x00, // DIB header size (124 bytes)
      static_cast<unsigned char>(width & 255u), static_cast<unsigned char>((width >> 8u) & 255u), 0x00,
      0x00, // Width (4px)
      static_cast<unsigned char>(height & 255u), static_cast<unsigned char>((height >> 8u) & 255u), 0x00,
      0x00, // Height (2px)
      0x01, 0x00,             // Planes (1)
      0x20, 0x00,             // Bits per pixel (32)
      0x03, 0x00, 0x00, 0x00, // Format (bitfield = use bitfields | no compression)
      0x20, 0x00, 0x00, 0x00, // Image raw size (32 bytes)
      0x13, 0x0B, 0x00, 0x00, // Horizontal print resolution (2835 = 72dpi * 39.3701)
      0x13, 0x0B, 0x00, 0x00, // Vertical print resolution (2835 = 72dpi * 39.3701)
      0x00, 0x00, 0x00, 0x00, // Colors in palette (none)
      0x00, 0x00, 0x00, 0x00, // Important colors (0 = all)
      0x00, 0x00, 0xFF, 0x00, // R bitmask (00FF0000)
      0x00, 0xFF, 0x00, 0x00, // G bitmask (0000FF00)
      0xFF, 0x00, 0x00, 0x00, // B bitmask (000000FF)
      0x00, 0x00, 0x00, 0xFF, // A bitmask (FF000000)
      0x42, 0x47, 0x52, 0x73, // sRGB color space
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, // Unused R, G, B entries for color space
      0x00, 0x00, 0x00, 0x00, // Unused Gamma X entry for color space
      0x00, 0x00, 0x00, 0x00, // Unused Gamma Y entry for color space
      0x00, 0x00, 0x00, 0x00, // Unused Gamma Z entry for color space

      0x00, 0x00, 0x00, 0x00, // Unknown
      0x00, 0x00, 0x00, 0x00, // Unknown
      0x00, 0x00, 0x00, 0x00, // Unknown
      0x00, 0x00, 0x00, 0x00, // Unknown
    };
  std::ofstream output( path, std::ios_base::out | std::ios_base::binary );
  output.write( reinterpret_cast<char *>(bmpData), 138 );
  output.write( reinterpret_cast<char *>(data), width * height * 4 );
  output.close();
}
}

#ifdef DEBUG_GL_CALLS
#define HECTOR_RVIZ_OVERLAY_DEBUG_CLEAR_ERRORS() _clearErrors();
#define HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR(_call) \
_call;\
_checkError(#_call, __FILE__, __LINE__)
#else
#define HECTOR_RVIZ_OVERLAY_DEBUG_CLEAR_ERRORS() do {} while(false);
#define HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( _call ) _call
#endif

#endif //HECTOR_RVIZ_OVERLAY_GL_HELPERS_H
