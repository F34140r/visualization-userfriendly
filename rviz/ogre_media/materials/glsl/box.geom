#version 150

// Generates an axis-aligned box with a given size
// for each input vertex.

#ifdef WITH_DEPTH
  //include:
  void passDepth( vec4 pos );
#endif


uniform mat4 worldviewproj_matrix;
uniform vec4 size;

in gl_PerVertex {
	vec4 gl_Position;
	vec4 gl_FrontColor;
	vec4 gl_TexCoord[];
} gl_in[];


out vec4 gl_Position;
out vec4 gl_FrontColor;
out vec4 gl_TexCoord[];

layout(points) in;
layout(triangle_strip, max_vertices=24) out;


const vec4 axes[3] = vec4[] ( 
  vec4(1.0,0.0,0.0,0.0), 
  vec4(0.0,1.0,0.0,0.0), 
  vec4(0.0,0.0,1.0,0.0)	);

const float lightness[6] = float[] ( 
    0.9, 0.5, 0.6, 0.6, 1.0, 0.4 );


void emitVertex( int side, vec4 x, vec4 y, vec4 z, vec3 tex )
{
  vec4 pos_rel = tex.x*x + tex.y*y + tex.z*z;
  vec4 pos = gl_in[0].gl_Position + vec4( pos_rel * size * 0.5 );
  gl_Position = worldviewproj_matrix * pos;
  gl_TexCoord[0] = vec4( tex.x*0.5+0.5, tex.y*0.5+0.5, 0.0, 0.0 );
  
#ifdef WITH_LIGHTING
    gl_FrontColor = vec4( gl_in[0].gl_FrontColor.xyz * lightness[side], 1.0 );
#else
    gl_FrontColor = vec4( gl_in[0].gl_FrontColor.xyz, 1.0 );
#endif

#ifdef WITH_DEPTH
  passDepth( pos );
#endif

  EmitVertex();
}


void main() 
{
  for( int side=0; side<3; side++ )
  {
    int side2 = (side+1)%3;
    int side3 = (side+2)%3;
    vec4 x=axes[ side ];
    vec4 y=axes[ side2 ];
    vec4 z=axes[ side3 ];
    
    // face for +z
    emitVertex( side, x, y, z, vec3(-1, -1, +1) );
    emitVertex( side, x, y, z, vec3(+1, -1, +1) );
    emitVertex( side, x, y, z, vec3(-1, +1, +1) );
    emitVertex( side, x, y, z, vec3(+1, +1, +1) );
    EndPrimitive();

    // face for -z
    emitVertex( side+3, x, y, z, vec3(-1, -1, -1) );
    emitVertex( side+3, x, y, z, vec3(-1, +1, -1) );
    emitVertex( side+3, x, y, z, vec3(+1, -1, -1) );
    emitVertex( side+3, x, y, z, vec3(+1, +1, -1) );
    EndPrimitive();
  }
}

