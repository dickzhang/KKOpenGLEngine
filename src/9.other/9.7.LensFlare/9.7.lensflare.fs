#version 330 core

in vec2 varTexCoord0;
uniform vec2 iMouse;
in vec2 texCoord;
out vec4 FragColor;

float rnd(vec2 p)
{
    float f = fract(sin(dot(p, vec2(12.1234, 72.8392) )*45123.2));
    return f;   
}

float rnd(float w)
{
    float f = fract(sin(w)*1000.);
    return f;   
}

float regShape(vec2 p, int N)
{
    float f;
    float a=atan(p.x,p.y)+.2;
    float b=6.28319/float(N);
    f=smoothstep(.5,.51, cos(floor(.5+a/b)*b-a)*length(p.xy));
    return f;
}
vec3 circle(vec2 p, float size, float decay, vec3 color,vec3 color2, float dist, vec2 mouse)
{
    float l = length(p + mouse*(dist*4.))+size/2.;
    float l2 = length(p + mouse*(dist*4.))+size/3.;
    ///these are circles, big, rings, and  tiny respectively
    float c = max(00.01-pow(length(p + mouse*dist), size*1.4), 0.0)*50.;
    float c1 = max(0.001-pow(l-0.3, 1./40.)+sin(l*30.), 0.0)*3.;
    float c2 =  max(0.04/pow(length(p-mouse*dist/2. + 0.09)*1., 1.), 0.0)/20.;
    float s = max(00.01-pow(regShape(p*5. + mouse*dist*5. + 0.9, 6) , 1.), 0.0)*5.;
   	color = 0.5+0.5*sin(color);
    color = cos(vec3(0.44, .24, .2)*8. + dist*4.)*0.5+.5;
 	vec3 f = c*color ;
    f += c1*color;
    f += c2*color;  
    f +=  s*color;
    return f-0.01;
}

float sun(vec2 p, vec2 mouse)
{
    float f;
    vec2 sunp = p+mouse;
    float sun = 1.0-length(sunp)*8.;
    return f;
}

void main()
{
    vec2 uv=texCoord-0.5;
    vec2 mm=iMouse;
    vec3 circColor = vec3(0.9, 0.2, 0.1);
    vec3 circColor2 = vec3(0.3, 0.1, 0.9);
    vec3 color = mix(vec3(0.3, 0.2, 0.02)/0.9, vec3(0.2, 0.5, 0.8), uv.y)*3.0;
    for(float i=0.;i<10.;i++)
    {
        color += circle(uv, pow(rnd(i*2000.)*1.8, 2.)+1.41, 0.0, circColor+i , circColor2+i, rnd(i*20.)*3.+0.2-.5, mm);
    }
    float a = atan(uv.y-mm.y, uv.x-mm.x);
    float l = max(1.0-length(uv-mm)-0.84, 0.0);
    float bright = 0.1;
    color += max(0.1/pow(length(uv-mm)*5., 5.), 0.0)*abs(sin(a*5.+cos(a*9.)))/20.;
    color += max(0.1/pow(length(uv-mm)*10., 1./20.), .0)+abs(sin(a*3.+cos(a*9.)))/8.*(abs(sin(a*9.)))/1.;
    color += (max(bright/pow(length(uv-mm)*4., 1./2.), 0.0)*4.)*vec3(0.2, 0.21, 0.3)*4.;
    color*= exp(1.0-length(uv-mm))/5.;
	FragColor = vec4(color,1.0);
}