#include <GL/glut.h>  
  
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")   
  
  
/* 
������ 
GL_TRIANGLES(�������һ��������) 
GL_TRIANGLE_STRIP(���������һ��������) 
GL_TRIANGLE_FAN(ͬ�ڶ�������2���͵�һ����������) 
 
�ı��� 
GL_QUADS(�ĵ�һ��) 
GL_QUAD_STRIP(�����ĵ�һ��) 
 
����� 
GL_POLYGON 
 
glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); 
*/  
void drawPoint(void)  
{  
    int i;  
      
    glColor3f (0.0, 0.0, 1.0);  
    glPointSize(5);  
    glBegin(GL_POINTS);  
    for (i = 0; i < 7; i++)  
        glVertex2f (0.2 + ((GLfloat) i * 0.1), 0.6);  
    glEnd ();  
}  
  
void drawLines(void)  
{  
    glColor3f(0.0,0.0,1.0);  
    glLineWidth(5);  
    glBegin(GL_LINES);  
        glVertex2f(0.0, 0.0);  
        glVertex2f(0.25,0.25);  
        glVertex2f(0.75,0.25);  
        glVertex2f(0.0,0.0);  
    glEnd();  
}  
  
void drawOneLine(GLfloat a,GLfloat b,GLfloat c,GLfloat d)  
{  
    glBegin(GL_LINES);  
        glVertex2f(a,b);  
        glVertex2f(c,d);  
    glEnd();  
}  
  
void drawLineStripple(void)  
{  
    glEnable (GL_LINE_STIPPLE);  
    glColor3f (0.0, 0.0, 1.0);  
    glLineStipple (1, 0x0101);  /*  dotted  */  
    drawOneLine (-0.25, 0.8, -0.75, 0.8);  
    glLineStipple (1, 0x00FF);  /*  dashed  */  
    drawOneLine (-0.25, 0.6, -0.75, 0.6);  
    glLineStipple (1, 0x1C47);  /*  dash/dot/dash  */  
    drawOneLine (-0.25, 0.4, -0.75, 0.4);  
      
    ///* in 2nd row, 3 wide lines, each with different stipple */  
    glLineWidth (5.0);  
    glLineStipple (1, 0x0101);  /*  dotted  */  
    drawOneLine (-0.25, 0.2, -0.75, 0.2);  
    glLineStipple (1, 0x00FF);  /*  dashed  */  
    drawOneLine (-0.25, 0.0, -0.75, 0.0);  
    glLineStipple (1, 0x1C47);  /*  dash/dot/dash  */  
    drawOneLine (-0.25, -0.2, -0.75, -0.2);  
    glLineWidth (1.0);  
    ///* in 4th row, 6 independent lines with same stipple  */  
    int i;  
    for (i = 0; i < 6; i++) {  
        drawOneLine (0.05 + ((GLfloat) i * 0.01), 0.5,  
            0.05 + ((GLfloat)(i+1) * 0.01), 0.5);  
    }  
    //  
    ///* in 5th row, 1 line, with dash/dot/dash stipple    */  
    ///* and a stipple repeat factor of 5                  */  
    glLineStipple (5, 0x1C47);  /*  dash/dot/dash  */  
    drawOneLine (50.0, 25.0, 350.0, 25.0);  
      
    glDisable (GL_LINE_STIPPLE);  
      
}  
  
void drawLineStrip()  
{  
       glColor3f (0.0, 0.0, 1.0);  
       glPointSize(5);  
      // glBegin(GL_LINE_STRIP);  
       glBegin(GL_LINE_LOOP);  
       glVertex2f(0.0,0.0);  
       glVertex2f(-0.3,-0.5);  
       glVertex2f(0.3,-0.5);  
       glEnd ();  
}  
  
void myDisplay(void)  
{  
    glClear(GL_COLOR_BUFFER_BIT);  
    // glRectf(-0.5f, -0.5f, 0.5f, 0.5f);  
      
    //������ɫ  
    glColor3f(1.0,1.0,0.0);  
      
    //��������  
    glBegin(GL_POLYGON);  
        glVertex3f(0.25,0.25,0.25);  
        glVertex3f(0.75,0.25,0.0);  
        glVertex3f(0.75,0.75,0.0);  
        glVertex3f(0.25,0.75,0.0);  
    //glVertex3f(0.0,0.5,0.0);  
    glEnd();  
    drawPoint();  
    drawLines();  
    drawLineStripple();  
    drawLineStrip();  
    glFlush();  
}  
  
void reshape (int w, int h)  
{  
    // glViewport (0, 0, (GLsizei) w, (GLsizei) h);  
    glMatrixMode (GL_PROJECTION);  
    glLoadIdentity ();  
    //gluOrtho2D (0.0, (GLdouble) w, 0.0, (GLdouble) h);  
    // glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);  
    //glOrtho(left, right, bottom, top, near, far)  
}  
  
int main(int argc, char *argv[])  
{  
    glutInit(&argc, argv);  
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);  
    glutInitWindowPosition(100, 100);  
    glutInitWindowSize(400, 400);  
    glutCreateWindow("��һ��OpenGL����");  
    //glClearColor (0.0, 1.0, 1.0, 0.0);  
    glutDisplayFunc(myDisplay);  
    glutReshapeFunc(reshape);  
    glutMainLoop();  
    return 0;  
}  