/*
 * Copyright (C) 2017  Stefan Fabian
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

#define GLEW_STATIC

#include <GL/glew.h>
#include "hector_rviz_overlay/render/glx_overlay_renderer.h"

#include "hector_rviz_overlay/render/qopengl_wrapper.h"

#include <OgreRenderQueueListener.h>
#include <OgreRenderWindow.h>

#include <rviz/display_context.h>
#include <rviz/render_panel.h>

#include <ros/ros.h>

#include "./gl_helpers.h"
#include <GL/glx.h>

namespace hector_rviz_overlay
{

namespace
{
const char *VERTEX_SHADER = R"LIT(
#version 130
in vec3 pos;
in vec2 coord;

out vec2 texCoord;

void main()
{
 gl_Position = vec4(pos, 1.0);   //Just output the incoming vertex
 texCoord = coord;
}
)LIT";

const char *FRAGMENT_SHADER = R"LIT(
#version 130
uniform sampler2D tex;
in vec2 texCoord;

void main()
{
  gl_FragColor = texture(tex, texCoord);
     // this fragment shader just sets the output color
     // to opaque red (red = 1.0, green = 0.0, blue = 0.0,
     // alpha = 1.0)
}
)LIT";

GLuint createShaderProgram()
{
  GLuint vertex_shader, fragment_shader;
  // Create the vertex and fragment shader
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( vertex_shader = glCreateShader( GL_VERTEX_SHADER ));
  if ( vertex_shader == 0 )
  {
    ROS_ERROR( "OverlayManager: Failed to create vertex shader!" );
    return 0;
  }
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( fragment_shader = glCreateShader( GL_FRAGMENT_SHADER ));
  if ( fragment_shader == 0 )
  {
    ROS_ERROR( "OverlayManager: Failed to create fragment shader!" );
    return 0;
  }

  // Compile vertex shader
  glShaderSource( vertex_shader, 1, &VERTEX_SHADER, nullptr );
  glCompileShader( vertex_shader );
  // Check result
  GLint result;
  glGetShaderiv( vertex_shader, GL_COMPILE_STATUS, &result );
  if ( result != GL_TRUE )
  {
    GLint length;
    glGetShaderiv( vertex_shader, GL_INFO_LOG_LENGTH, &length );
    char message[length + 1];
    glGetShaderInfoLog( vertex_shader, length, nullptr, message );
    ROS_ERROR( "OverlayManager: Failed to compile vertex shader!\nMessage: %s", message );
    return 0;
  }

  // Compile fragment shader
  glShaderSource( fragment_shader, 1, &FRAGMENT_SHADER, nullptr );
  glCompileShader( fragment_shader );
  // Check result
  glGetShaderiv( fragment_shader, GL_COMPILE_STATUS, &result );
  if ( result != GL_TRUE )
  {
    GLint length;
    glGetShaderiv( fragment_shader, GL_INFO_LOG_LENGTH, &length );
    char message[length + 1];
    glGetShaderInfoLog( fragment_shader, length, nullptr, message );
    ROS_ERROR( "OverlayManager: Failed to compile fragment shader!\nMessage: %s", message );
    return 0;
  }

  // Create program
  GLuint program = glCreateProgram();
  glAttachShader( program, vertex_shader );
  glAttachShader( program, fragment_shader );
  glLinkProgram( program );

  // Check result
  glGetProgramiv( program, GL_LINK_STATUS, &result );
  if ( result != GL_TRUE )
  {
    GLint length;
    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &length );
    char message[length + 1];
    glGetProgramInfoLog( program, length, nullptr, message );
    ROS_ERROR( "OverlayManager: Failed to link shader program!\nMessage: %s", message );
    return 0;
  }

  // Clean up
  glDetachShader( program, vertex_shader );
  glDetachShader( program, fragment_shader );

  glDeleteShader( vertex_shader );
  glDeleteShader( fragment_shader );

  return program;
}
}

class GLXOverlayRenderer::RenderTargetListener : public Ogre::RenderTargetListener
{
public:
  explicit RenderTargetListener( GLXOverlayRenderer *renderer ) : renderer_( renderer ) { }

  void postRenderTargetUpdate( const Ogre::RenderTargetEvent &evt ) override
  {
    renderer_->render();
  }

private:
  GLXOverlayRenderer *renderer_;
};

GLXOverlayRenderer::GLXOverlayRenderer( rviz::DisplayContext *context )
  : OverlayRenderer( context )
{
  scene_manager_ = context_->getSceneManager();
  Ogre::MaterialPtr clearMat = Ogre::MaterialManager::getSingleton().getByName( "BaseWhite" );
  clear_pass_ = clearMat->getTechnique( 0 )->getPass( 0 );

  render_target_listener_ = new RenderTargetListener( this );
  render_panel_->getRenderWindow()->addListener( render_target_listener_ );
}

GLXOverlayRenderer::~GLXOverlayRenderer()
{
  render_panel_->getRenderWindow()->removeListener( render_target_listener_ );

  delete qopengl_wrapper_;

  delete render_target_listener_;
  delete pixel_data_;
}

void GLXOverlayRenderer::initialize()
{
  if ( qopengl_wrapper_ != nullptr ) return;
  Display *display;
  render_panel_->getRenderWindow()->getCustomAttribute( "DISPLAY", &display );
  qopengl_wrapper_ = new QOpenGLWrapper( display, geometry_.size());
  framebuffer_object_updated_ = true;
}

void GLXOverlayRenderer::initializeOpenGL()
{
  if ( vertex_array_object_ != 0 ) return;

  glewInit();

  glGenVertexArrays( 1, &vertex_array_object_ );
  glBindVertexArray( vertex_array_object_ );

  const GLfloat vertices[25] = {
    // Positions        // Texture coordinates
    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f
  };

  glGenBuffers( 1, &vertex_buffer_object_ );
  glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer_object_ );
  glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

  shader_program_ = createShaderProgram();
  if ( shader_program_ == 0 )
  {
    ROS_ERROR( "OverlayManager: Shader creation failed! Overlays won't be visible!" );
    return;
  }
  glBindVertexArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  const GLubyte *vendor = glGetString( GL_VENDOR );
  if ( strncasecmp( reinterpret_cast<const char *>(vendor), "Intel", 5 ) == 0 )
  {
    can_share_texture_ = false; // Dirty workaround because the check doesn't work on 18.04 but sharing also doesnt
  }
  else
  {
    // Determine if texture sharing is supported
    qopengl_wrapper_->setSize( { 4, 4 } );
    qopengl_wrapper_->makeCurrent();
    // Make sure the fbo is created
    qopengl_wrapper_->prepareRender();
    qopengl_wrapper_->finishRender();
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, qopengl_wrapper_->texture());
    const unsigned int test_texture_content[16] = {
      0xdeadbeef, 0xfadefade, 0xff0000ff, 0x00ff00ff,
      0x0000ffff, 0xffff0088, 0xff00ff88, 0xf0f000ff,
      0x8f8f8fff, 0x12345678, 0xfedcba98, 0x11224488,
      0x888888ff, 0xffffff88, 0x00000000, 0xffffffff
    };
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *) test_texture_content );
    glBindTexture( GL_TEXTURE_2D, 0 );
    qopengl_wrapper_->doneCurrent();

    glActiveTexture( GL_TEXTURE0 );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, qopengl_wrapper_->texture());
    unsigned int texture_content[16];
    glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *) texture_content );
    can_share_texture_ = true;
    for ( int i = 0; i < 16; ++i )
    {
      if ( test_texture_content[i] == texture_content[i] ) continue;
      can_share_texture_ = false;
      break;
    }
  }

  if ( can_share_texture_ ) return;
  ROS_INFO(
    "OverlayManager: Looks like texture sharing isn't working on your system. Falling back to texture copying." );

  // If we can't share textures, we have to generate one
  glActiveTexture( GL_TEXTURE0 );
  glGenTextures( 1, &texture_ );
  glBindTexture( GL_TEXTURE_2D, texture_ );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}


void GLXOverlayRenderer::releaseResources()
{
  if ( qopengl_wrapper_ == nullptr ) return;
  render_panel_->getRenderWindow()->removeListener( render_target_listener_ );

  if ( texture_ != 0 )
  {
    HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDeleteTextures( 1, &texture_ ));
    texture_ = 0;
  }

  if ( shader_program_ != 0 )
  {
    HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDeleteProgram( shader_program_ ));
    shader_program_ = 0;
  }

  if ( vertex_array_object_ != 0 )
  {
    HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDeleteVertexArrays( 1, &vertex_array_object_ ));
    vertex_array_object_ = 0;
  }

  if ( vertex_buffer_object_ != 0 )
  {
    HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDeleteBuffers( 1, &vertex_buffer_object_ ));
    vertex_buffer_object_ = 0;
  }

  qopengl_wrapper_->makeCurrent();
  for ( auto &overlay : overlays_ )
  {
    overlay->releaseRenderResources();
  }
  overlays_.clear();
  qopengl_wrapper_->doneCurrent();

  delete qopengl_wrapper_;
  qopengl_wrapper_ = nullptr;
}

void GLXOverlayRenderer::prepareOverlay( OverlayPtr &overlay )
{
  qopengl_wrapper_->makeCurrent();
  OverlayRenderer::prepareOverlay( overlay );
  qopengl_wrapper_->doneCurrent();
}

void GLXOverlayRenderer::releaseOverlay( OverlayPtr &overlay )
{
  qopengl_wrapper_->makeCurrent();
  OverlayRenderer::releaseOverlay( overlay );
  qopengl_wrapper_->doneCurrent();
}

void GLXOverlayRenderer::hide()
{
  // nothing to do
}

void GLXOverlayRenderer::redrawLastFrame()
{
  HECTOR_RVIZ_OVERLAY_DEBUG_CLEAR_ERRORS();
  // Save and reset matrices
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPushMatrix());
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glLoadIdentity());

  scene_manager_->getDestinationRenderSystem()->_setWorldMatrix( Ogre::Matrix4::IDENTITY );
  scene_manager_->getDestinationRenderSystem()->_setViewMatrix( Ogre::Matrix4::IDENTITY );
  scene_manager_->getDestinationRenderSystem()->_setProjectionMatrix( Ogre::Matrix4::IDENTITY );

  //Set a clear pass to give the renderer a clear renderstate
  scene_manager_->_setPass( clear_pass_, true, false );

  if ( vertex_array_object_ == 0 )
  {
    HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPushAttrib( GL_ALL_ATTRIB_BITS ));
    initializeOpenGL();
    HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPopAttrib());
  }
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPushAttrib( GL_ALL_ATTRIB_BITS ));

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDisable( GL_DEPTH_TEST ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDisable( GL_CULL_FACE ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDisable( GL_LIGHTING ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glEnable( GL_BLEND ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ));

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glUseProgram( shader_program_ ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glUniform1i( glGetUniformLocation( shader_program_, "tex" ), 0 ));

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer_object_ ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR(
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), nullptr ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glEnableVertexAttribArray( 0 ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR(
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), (void *) (3 * sizeof( float ))));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glEnableVertexAttribArray( 1 ));

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glEnable( GL_TEXTURE_2D ));
  if ( can_share_texture_ )
  {
    glBindTexture( GL_TEXTURE_2D, qopengl_wrapper_->texture());
  }
  else
  {
    glBindTexture( GL_TEXTURE_2D, texture_ );
  }

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDisableVertexAttribArray( 0 ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDisableVertexAttribArray( 1 ));

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glUseProgram( 0 ));
  glBindTexture( GL_TEXTURE_2D, 0 );

  // Restore attributes
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPopAttrib());

  // Restore matrices
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPopMatrix());
}

void GLXOverlayRenderer::prepareRender( int width, int height )
{
  HECTOR_RVIZ_OVERLAY_DEBUG_CLEAR_ERRORS();
  // Save and reset matrices
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPushMatrix());
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glLoadIdentity());

  scene_manager_->getDestinationRenderSystem()->_setWorldMatrix( Ogre::Matrix4::IDENTITY );
  scene_manager_->getDestinationRenderSystem()->_setViewMatrix( Ogre::Matrix4::IDENTITY );
  scene_manager_->getDestinationRenderSystem()->_setProjectionMatrix( Ogre::Matrix4::IDENTITY );

  //Set a clear pass to give the renderer a clear renderstate
  scene_manager_->_setPass( clear_pass_, true, false );

  if ( vertex_array_object_ == 0 )
  {
    HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPushAttrib( GL_ALL_ATTRIB_BITS ));
    initializeOpenGL();
    HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPopAttrib());
  }

  // Save attributes
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPushAttrib( GL_ALL_ATTRIB_BITS ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS ));

  qopengl_wrapper_->makeCurrent();
  if ( width != qopengl_wrapper_->size().width() || height != qopengl_wrapper_->size().height())
  {
    qopengl_wrapper_->setSize( { width, height } );
    framebuffer_object_updated_ = true;
    if ( !can_share_texture_ )
    {
      delete pixel_data_;
      pixel_data_ = new unsigned char[width * height * 4];
    }
  }
  qopengl_wrapper_->prepareRender();
}

void GLXOverlayRenderer::finishRender()
{
  framebuffer_object_updated_ = false;
  qopengl_wrapper_->finishRender();
  if ( !can_share_texture_ )
  {
    glBindTexture( GL_TEXTURE_2D, qopengl_wrapper_->texture());
    glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data_ );
  }
  qopengl_wrapper_->doneCurrent();
  HECTOR_RVIZ_OVERLAY_DEBUG_CLEAR_ERRORS();
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPopClientAttrib()); // Restore state
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPopAttrib());
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPushAttrib( GL_ALL_ATTRIB_BITS ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS ));

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDisable( GL_DEPTH_TEST ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDisable( GL_CULL_FACE ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDisable( GL_LIGHTING ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glEnable( GL_BLEND ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ));

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glUseProgram( shader_program_ ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glUniform1i( glGetUniformLocation( shader_program_, "tex" ), 0 ));

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer_object_ ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR(
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), nullptr ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glEnableVertexAttribArray( 0 ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR(
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), (void *) (3 * sizeof( float ))));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glEnableVertexAttribArray( 1 ));

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glEnable( GL_TEXTURE_2D ));
  if ( can_share_texture_ )
  {
    glBindTexture( GL_TEXTURE_2D, qopengl_wrapper_->texture());
  }
  else
  {
    glBindTexture( GL_TEXTURE_2D, texture_ );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, qopengl_wrapper_->size().width(), qopengl_wrapper_->size().height(), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, pixel_data_ );
  }

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDisableVertexAttribArray( 0 ));
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glDisableVertexAttribArray( 1 ));

  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glUseProgram( 0 ));
  glBindTexture( GL_TEXTURE_2D, 0 );

  // Restore attributes
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPopClientAttrib());
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPopAttrib());

  // Restore matrices
  HECTOR_RVIZ_OVERLAY_DEBUG_CHECK_GL_ERROR( glPopMatrix());
}

QPaintDevice *GLXOverlayRenderer::paintDevice() noexcept
{
  return qopengl_wrapper_->paintDevice();
}

QOpenGLContext *GLXOverlayRenderer::context()
{
  return qopengl_wrapper_->context();
}

QOpenGLFramebufferObject *GLXOverlayRenderer::framebufferObject()
{
  return qopengl_wrapper_->framebufferObject();
}

bool GLXOverlayRenderer::framebufferObjectUpdated() noexcept
{
  return framebuffer_object_updated_;
}
}
