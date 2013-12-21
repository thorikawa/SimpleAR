#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <glut.h>
#include "MarkerDetector.hpp"

#define WINDOW_NAME "ar"
#define WIDTH 640
#define HEIGHT 480
#define CAMERA_WIDTH WIDTH
#define CAMERA_HEIGHT HEIGHT

using namespace std;
using namespace cv;

GLfloat lineX[] = { 0, 0, 0, 1, 0, 0 };
GLfloat lineY[] = { 0, 0, 0, 0, 1, 0 };
GLfloat lineZ[] = { 0, 0, 0, 0, 0, 1 };
GLuint backgroundTextureId;
const GLfloat squareVertices[] = {
    -0.5f, -0.5f,
    0.5f,  -0.5f,
    -0.5f,  0.5f,
    0.5f,   0.5f,
};
const GLubyte squareColors[] = {
    255, 255,   0, 255,
    0,   255, 255, 255,
    0,     0,   0,   0,
    255,   0, 255, 255,
};

Mat frame;
VideoCapture capture;

CameraCalibration calib = CameraCalibration(
        6.24860291e+02 * (640. / 352.), 6.24860291e+02 * (480. / 288.),
        640 * 0.5f, 480 * 0.5f);
MarkerDetector markerDetector = MarkerDetector(calib);
Matrix44 transMat;

bool findMarker = false;

void buildProjectionMatrix (Matrix33 cameraMatrix, Matrix44& projectionMatrix) {
    GLfloat w = (GLfloat) WIDTH;
    GLfloat h = (GLfloat) HEIGHT;

    float near = 0.01;  // Near clipping distance
    float far  = 100;  // Far clipping distance
    
    // Camera parameters
    float f_x = cameraMatrix.data[0]; // Focal length in x axis
    float f_y = cameraMatrix.data[4]; // Focal length in y axis (usually the same?)
    float c_x = cameraMatrix.data[2]; // Camera primary point x
    float c_y = cameraMatrix.data[5]; // Camera primary point y

    float size = 2.0;
    projectionMatrix.data[0] = - 2.0 * f_x / w;
    projectionMatrix.data[1] = 0.0;
    projectionMatrix.data[2] = 0.0;
    projectionMatrix.data[3] = 0.0;
    
    projectionMatrix.data[4] = 0.0;
    projectionMatrix.data[5] = 2.0 * f_y / h;
    projectionMatrix.data[6] = 0.0;
    projectionMatrix.data[7] = 0.0;
    
    projectionMatrix.data[8] = size * c_x / w - 1.0;
    projectionMatrix.data[9] = size * c_y / h - 1.0;
    projectionMatrix.data[10] = -( far + near ) / ( far - near );
    projectionMatrix.data[11] = -1.0;
    
    projectionMatrix.data[12] = 0.0;
    projectionMatrix.data[13] = 0.0;
    projectionMatrix.data[14] = - 2.0 * far * near / ( far - near );
    projectionMatrix.data[15] = 0.0;
}

void drawBackground () {
    GLfloat w = (GLfloat) WIDTH;
    GLfloat h = (GLfloat) HEIGHT;
    
    const GLfloat squareVertices[] =
    {
        0, 0,
        w, 0,
        0, h,
        w, h
    };
    
     static const GLfloat textureVertices[] =
     {
        1, 0,
        1, 1,
        0, 0,
        0, 1
     };
    
    static const GLfloat proj[] =
    {
        0, -2.f/w, 0, 0,
        -2.f/h, 0, 0, 0,
        0, 0, 1, 0,
        1, 1, 0, 1
    };
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glDepthMask(false);
    glDisable(GL_COLOR_MATERIAL);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backgroundTextureId);
    
    // Update attribute values.
    glVertexPointer(2, GL_FLOAT, 0, squareVertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, textureVertices);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glColor4f(1,1,1,1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

void drawAR () {
    Matrix33 cameraMatrix = calib.getIntrinsic();
    Matrix44 proj;
    buildProjectionMatrix(cameraMatrix, proj);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj.data);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glPushMatrix();
    glLineWidth(3.0f);

    if (findMarker) {
        glLoadMatrixf(reinterpret_cast<const GLfloat*>(&transMat.data[0]));

        glVertexPointer(2, GL_FLOAT, 0, squareVertices);
        glEnableClientState(GL_VERTEX_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, squareColors);
        glEnableClientState(GL_COLOR_ARRAY);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableClientState(GL_COLOR_ARRAY);

        float scale = 0.5;
        glScalef(scale, scale, scale);
        
        glTranslatef(0, 0, 0.1f);

        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        glVertexPointer(3, GL_FLOAT, 0, lineX);
        glDrawArrays(GL_LINES, 0, 2);
        
        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
        glVertexPointer(3, GL_FLOAT, 0, lineY);
        glDrawArrays(GL_LINES, 0, 2);
        
        glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
        glVertexPointer(3, GL_FLOAT, 0, lineZ);
        glDrawArrays(GL_LINES, 0, 2);
    }

    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
}

void display (void) {
    // printf("display\n");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();
    drawAR();
    glutSwapBuffers();
}

void idle (void) {
    // printf("idle\n");
    capture >> frame;
    cvtColor(frame, frame, CV_BGR2RGBA);

    markerDetector.processFrame(frame);
    std::vector<Transformation> transformations = markerDetector.getTransformations();
    if (transformations.size() > 0) {
        Transformation transformation = transformations[0];
        transMat = transformation.getMat44();
        findMarker = true;
    } else {
        findMarker = false;
    }
    printf("%lu\n", transformations.size());

    // ###debug###
    // Matrix44 tmp = { 0.04455F, 0.89660F, -0.44060F, 0.00000F, -0.98208F, -0.04154F, -0.18383F, 0.00000F, -0.18312F,
    //            0.44090F, 0.87868F, 0.00000F, 1.21119F, 0.98513F, -9.28195F, 1.00000F };
    // transMat = tmp;
    // findMarker = true;

    // update background texture
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, backgroundTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.cols, frame.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame.data);
    
    int glErrorCode = glGetError();
    if (glErrorCode != GL_NO_ERROR) {
        cerr << glErrorCode << std::endl;
    }

    glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y) {
    if (key == '\x1b') {
        exit(-1);
    }
}

int main (int argc, char* argv[]) {
    {
        capture = VideoCapture(0);
        capture.set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH);
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT);
    }

    {
        glutInit(&argc, argv);
        glutCreateWindow(WINDOW_NAME);
        glutInitWindowSize(WIDTH, HEIGHT);
        glutInitWindowPosition(100, 100);
        // glutFullScreen();
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
        glutDisplayFunc(display);
        glutIdleFunc(idle);
        glutKeyboardFunc(keyboard);
    }
    
    // setup background texture
    {
        glGenTextures(1, &backgroundTextureId);
        glBindTexture(GL_TEXTURE_2D, backgroundTextureId);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // This is necessary for non-power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glEnable(GL_DEPTH_TEST);
    }

    glutMainLoop();
    return 0;
}
