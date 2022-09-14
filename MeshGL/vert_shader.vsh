attribute highp vec4 qt_Vertex;
attribute highp vec3 qt_Normal;
attribute highp vec3 qt_Color;
uniform highp mat4 qt_ModelViewProjectionMatrix;
varying highp vec3 qt_fragColor;

void main(void)
{
    gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;
    qt_fragColor = qt_Color;
}
