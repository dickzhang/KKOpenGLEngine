#version 330 core

uniform float VirtualTextureSize;
uniform float AtlasScale;
uniform float BorderScale;
uniform float BorderOffset;
uniform float MipBias;
uniform float PageTableSize;

uniform sampler2D s_vt_page_table;
uniform sampler2D s_vt_texture_atlas;

in vec2 v_texcoord0;
out vec4 FragColor;

// This function estimates mipmap levels
float MipLevel( vec2 uv, float size )
{
   vec2 dx = dFdx( uv * size );
   vec2 dy = dFdy( uv * size );
   float d = max( dot( dx, dx ), dot( dy, dy ) );

   return max( 0.5 * log2( d ), 0 );
}

// This function samples the page table and returns the page's 
// position and mip level. 
vec3 SampleTable( vec2 uv, float mip )
{
   vec2 offset = fract( uv * PageTableSize ) / PageTableSize;
   return textureLod( s_vt_page_table, uv - offset, mip ).xyz;
}

// This functions samples from the texture atlas and returns the final color
vec4 SampleAtlas( vec3 page, vec2 uv )
{
   float mipsize = exp2( floor( page.z * 255.0 + 0.5 ) );

   uv = fract( uv * PageTableSize / mipsize );

   uv *= BorderScale;
   uv += BorderOffset;

   vec2 offset = floor( page.xy * 255.0 + 0.5 );

   return texture( s_vt_texture_atlas, ( offset + uv ) * AtlasScale );
}

// Ugly brute force trilinear, look up twice and mix
vec4 VirtualTextureTrilinear( vec2 uv )
{
   float miplevel = MipLevel( uv, VirtualTextureSize );
   miplevel = clamp( miplevel, 0.0, log2( PageTableSize )-1.0 );

   float mip1     = floor( miplevel );
   float mip2    = mip1 + 1.0;
   float mipfrac  = miplevel - mip1;

   vec3 page1 = SampleTable( uv, mip1 );
   vec3 page2 = SampleTable( uv, mip2 );

   vec4 sample1 = SampleAtlas( page1, uv );
   vec4 sample2 = SampleAtlas( page2, uv );

   return mix( sample1, sample2, mipfrac );
}

// Simple bilinear
vec4 VirtualTexture( vec2 uv )
{
   float mip = floor( MipLevel( uv, VirtualTextureSize ) );
   mip = clamp( mip, 0, log2( PageTableSize ) );

   vec3 page = SampleTable( uv, mip );
   return SampleAtlas( page, uv );
}

void main()
{
	FragColor = VirtualTexture(v_texcoord0.xy);
}