varying highp vec3 qt_fragColor;

void main(void)
{
    gl_FragColor = vec4(qt_fragColor, 1);
}
