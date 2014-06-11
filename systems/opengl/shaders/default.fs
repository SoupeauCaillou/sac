#ifdef GL_ES
precision lowp float;
#endif
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform vec4 vColor;

varying vec2 uvVarying;

void main()
{
    // Fetch color from tex0, alpha from tex1 (in red channel)
    vec4 color = vec4(
        texture2D(tex0, uvVarying).rgb,
        texture2D(tex1, uvVarying).r);

    // premultiplied alpha
    color.rgb *= vColor.a;

    gl_FragColor = color * vColor;
}

