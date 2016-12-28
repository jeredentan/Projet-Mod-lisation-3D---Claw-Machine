
#include <QtWidgets>
#include <QtOpenGL>
#include <QGLWidget>
#include "glu.h"
#include "gl.h"
#include "opencv2/opencv.hpp"
#include <time.h>
#include <ctime>
#include <vector>
#include <QString>
#include "myglwidget.h"
#define PI 3.14159265
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
using namespace std;
using namespace cv;
int i=0;
MyGLWidget::MyGLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{

    timer_animation_finale=new QTimer(this);
    connect(timer_animation_finale,SIGNAL(timeout()),this,SLOT(animation_partie_gagnee()));

    partie_gagnee=false;
    partie_lancee=false;
    sphere_catched=false;
    descente=false;
    remonte=false;
    deplacement_trou=false;
    sphere_catched=false;
    update_sphere_done=false;

    xRot = 0;
    yRot = 0;
    zRot = 0;
    xPos=0;
    yPos=0;
    zPos=0;
    xsphere=-0.6;
    ysphere=-1.2;
    zsphere=0.;
    sphere_actuelle=0;

    repositionnement_temps_depasse=false;
    pince_reinitialisee=false;
}



MyGLWidget::~MyGLWidget()
{
}


QSize MyGLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize MyGLWidget::sizeHint() const
{
    return QSize(400, 400);
}

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360)
        angle -= 360 * 16;
}

void MyGLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != xRot) {
        xRot = angle;

        emit xRotationChanged(angle);
        updateGL();
    }
}

void MyGLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != yRot) {
        yRot = angle;

        emit yRotationChanged(angle);
        updateGL();
    }
}

void MyGLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != zRot) {
        zRot = angle;

        emit zRotationChanged(angle);
        updateGL();
    }
}

void MyGLWidget::setXTranslation(double pos)
{
    if (pos !=xPos) {
        //std::cout<<"xPos :"<<xPos<<std::endl;
        xPos = xPos + pos/30;
        //Limites de la cage (-0.5,0.5), on laisse 0.1 de marge pour largeur bras+fixation plafond
        if(xPos<-0.4){
            xPos=-0.4;
        }
        if(xPos>0.4){
            xPos=0.4;
        }

        emit xTranslationChanged(pos);
        updateGL();
    }
}

void MyGLWidget::setYTranslation(double pos)
{
    if (pos !=yPos) {
        //  std::cout<<"yPos"<<yPos<<std::endl;
        yPos = yPos + pos/20;
        if(yPos<=-1.4){
            yPos=-1.4;
        }
        if(yPos>0){
            yPos=0;
        }

        emit yTranslationChanged(pos);
        updateGL();
    }
}

void MyGLWidget::setZTranslation(double pos)
{
    if (pos !=zPos) {
        //  std::cout<<"zPos"<<zPos<<std::endl;
        zPos = zPos+ pos/30;
        if(zPos<-0.4){
            zPos=-0.4;
        }
        if(zPos>0.40){
            zPos=0.40;
        }
        emit zTranslationChanged(pos);
        updateGL();
    }
}

void MyGLWidget::initializeGL()
{
    qglClearColor(Qt::black);
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    // glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    static GLfloat lightPosition[4] = { 0, 10, 0, 1.0};
    glLightfv(GL_LIGHT0, GL_LINEAR_ATTENUATION, lightPosition);
    texture[0]= loadTexture( ":/textures/textureplancher.jpg");
    texture[1]= loadTexture( ":/textures/Logo-TSE.png");
    texture[2]=loadTexture(":/textures/texture_brique.jpg");
    texture[3]=loadTexture(":/textures/texture_ecaille_metal.jpg");
    texture[4]=loadTexture(":/textures/texture_socle_bras.jpg");
    texture[5]=loadTexture(":/textures/texture_balle_tennis.jpg");

    // Appel des displayList
    createdisplaylists();
    GLdisplayList();

}
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0, 0,-2);

    glRotatef(xRot, 1.0, 0.0, 0.0);
    glRotatef(yRot, 0.0, 1.0, 0.0);
    glRotatef(zRot, 0.0, 0.0, 1.0);

    draw();
}

void MyGLWidget::placement_spheres(){

    glPushMatrix();
    glPushMatrix();
    glTranslatef(posX_sphereactuelle,-0.42,posZ_sphereactuelle);
    glScalef(0.1,0.1,0.1);
    glScalef(taille_sphere,taille_sphere,taille_sphere);
    glCallList(GLdisplayList_[0]);
    // glutSolidSphere(0.5,50,50);
    glPopMatrix();

    glPopMatrix();

}

void MyGLWidget::update_spheres_au_sol(){
    //Elimine la sphere attrapée du sol
    if(sphere_catched==true  && update_sphere_done==false){
        posX_sphereactuelle=-10000;
        posZ_sphereactuelle=-10000;
        update_sphere_done=true;
    }

}

void MyGLWidget::descendre_pince(int a){

    if(a==0){   //On scale le bras
        if(yPos<=-1.24 || remonte==true){       //Quand on arrive en bas, on remonte
            glScalef(1,1,-yPos+1);
            update_spheres_au_sol();
            remonte=true;
            if(yPos==0){                       //Quand on arrive en haut, on a 2 choix
                descente=false;
                if(sphere_catched==true){      //On a attrapé une sphere donc on se deplace vers le trou et on arrete de remonter
                    remonte=false;
                    deplacement_trou=true;
                }else{                         //On a rien attrapé donc on arrete simplement de remonter
                    remonte=false;
                }
            }
        }else{
            glScalef(1,1,-yPos+1);             // Tant qu'on est pas en bas, on descend (scale pour etendre le bras )
        }
    }
    if(a==1){   //ON translate la balle
        if(yPos<=-1.24){
            glTranslatef(0,0,-yPos*4);
            remonte=true;
            if(yPos==0){
                descente=false;
                if(sphere_catched==true){            //Si on a chopé une balle, on part vers le trou
                    deplacement_trou=true;

                }else{
                    remonte=false;
                }
            }
        }else{
            glTranslatef(0,0,-yPos*4);            // Tant qu'on est pas en bas, on descend (translate pour faire descendre la sphere)
        }

    }


}

void MyGLWidget::mouvement_verticaux_brasrobot(){
        if(remonte==true){
            if(sphere_catched==true){
                glTranslatef(0,0,(-yPos/6)+1.4);
            }
            glScalef(1/0.17,1/0.17,1/0.17);
            glScalef(2,2,2);
            draw_balle_dans_main();
        }else{
            if(lacher_balle==true){
                glTranslatef(0,0,-yPos*8);
                glScalef(1/0.17,1/0.17,1/0.17);
                glScalef(2,2,2);
                //   std::cout<<"ON DRAW BALLE EN DEPLACEMENT"<<std::endl;
                draw_balle_dans_main();
                if(yPos==-1.4){
                    yPos=0;
                    lacher_balle=false;
                    sphere_catched=false;
                    remonte=false;
                    descente=false;
                    deplacement_trou=false;
                    update_sphere_done=false;
                    partie_lancee=false;
                    partie_gagnee=true;
                }
            }else{
                if(deplacement_trou==true && repositionnement_temps_depasse==false){
                    glTranslatef(0,0,+1.5);
                    glScalef(1/0.17,1/0.17,1/0.17);
                    glScalef(2,2,2);
                    draw_balle_dans_main();
                }
            }
        }

}

void MyGLWidget::mouvement_horizontaux_brasrobot(){

        if(deplacement_trou==true){
            remonte=false;
            glTranslatef(xPos,1,zPos);
            if(xPos>0.3 && zPos>0.3){
                deplacement_trou=false;
                if(repositionnement_temps_depasse==false){
                      lacher_balle=true;
                }else{
                    //Si on repositionne le bras en fin de partie
                        pince_reinitialisee=true;
                }

            }
        }else{
            glTranslatef(xPos,1,zPos);
        }


}

void MyGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    // Coordonnées globales vers coordonnées écran
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, ((float)width/(float)height), 0.01, 30);
    // Coordonnées locales vers coordonnées globales.
    glMatrixMode(GL_MODELVIEW);

}

void MyGLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 1 * dy);
        setZRotation(zRot + 1 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 1 * dy);
        setZRotation(zRot + 1 * dx);
    }

    lastPos = event->pos();
}

void MyGLWidget::GLdisplayList()
{
    // Sphere
    GLdisplayList_[0]= glGenLists(1); // dislpaylist pour la sphere
    glNewList( GLdisplayList_[0], GL_COMPILE );
    double pas = 10*PI/180;
    double rayon_sphere = .5;
    for (double a=0; a<PI; a+=pas) {
        glBegin(GL_QUAD_STRIP);
        glColor3f(1,1,1);
        for (double t=0; t<=2*PI+pas; t+=pas) {
            glVertex3f(rayon_sphere*cos(t)*sin(a), rayon_sphere*sin(t)*sin(a), rayon_sphere*cos(a));
            glVertex3f(rayon_sphere*cos(t)*sin(a+pas), rayon_sphere*sin(t)*sin(a+pas), rayon_sphere*cos(a+pas));
        }
        glEnd();
    }
    glEndList();
    // End Sphere

    // Cube
    GLdisplayList_[1]= glGenLists(1); // displaylist pour le cube
    glNewList( GLdisplayList_[1], GL_COMPILE );
    double nPas = PI/2;
    double rayon_cube = .5;
    glBegin(GL_QUAD_STRIP);
    double a = 0;
    for (double t = PI/4; t <= 2*PI + PI/4; t += nPas)
    {
        glTexCoord2f(a,1);
        glVertex3f(rayon_cube*cos(t),rayon_cube*sin(t),.5);
        glTexCoord2f(a,0);
        glVertex3f(rayon_cube*cos(t),rayon_cube*sin(t),-.5);
        a += 4*nPas/(2*PI);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0);
    for (double t = PI/4; t <= 2*PI + PI/4; t += nPas)
        glVertex3f(rayon_cube*cos(t),rayon_cube*sin(t),.5);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0);
    for (double t = 2*PI + PI/4; t >= PI/4; t -= nPas)
        glVertex3f(rayon_cube*cos(t),rayon_cube*sin(t),-.5);
    glEnd();
    glEndList();
    // End Cube

    // Disque
    GLdisplayList_[2]= glGenLists(1); // dislaylist pour le cylindre
    glNewList( GLdisplayList_[2], GL_COMPILE );
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0.5);
    for (double t=0; t<=2*PI+pas; t+=pas)
        glVertex3f(cos(t),sin(t),0.3);
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for(double t = 0; t <= 2*PI + pas; t += pas)
    {
        glVertex3f(cos(t), sin(t), 0);
        glVertex3f(cos(t), sin(t), 0.3);
    }
    glEnd();
    glEndList();
    //End Disque
    GLdisplayList_[4]= glGenLists(1);
    glNewList( GLdisplayList_[4], GL_COMPILE );

    glBegin(GL_QUADS);
    glScalef(0.5,.5,5);

    glVertex3f(0,0,0);
    glVertex3f (0,1,0);
    glVertex3f (1, 1, 0);
    glVertex3f (1, 0, 0);

    glVertex3f (0, 0, 1);
    glVertex3f (1, 0, 1);
    glVertex3f (1, 1, 1);
    glVertex3f (0, 1, 1);

    glVertex3f (0, 0, 0);
    glVertex3f (1, 0, 0);
    glVertex3f (1, 0, 1);
    glVertex3f (0, 0, 1);


    glVertex3f (0, 1, 0);
    glVertex3f (0, 1, 1);
    glVertex3f (1, 1, 1);
    glVertex3f (1, 1, 0);

    glVertex3f (1, 0, 0);
    glVertex3f (1, 1, 0);
    glVertex3f (1, 1, 1);
    glVertex3f (1, 0, 1);

    glVertex3f (0, 0, 0);
    glVertex3f (0, 0, 1);
    glVertex3f (0, 1, 1);
    glVertex3f (0, 1, 0);
    glEnd();
    glEndList();

}

void MyGLWidget::createdisplaylists(){

    displaylistbras[0]= glGenLists(1);
    displaylistbras[1]= glGenLists(1);
    displaylistbras[2]= glGenLists(1);
    displaylistbras[3]= glGenLists(1);
    displaylistbras[4]= glGenLists(1);

    glNewList (displaylistbras[0], GL_COMPILE );
    int cotes=360;
    double rayon=1;
    double rayon_trou=0.1;
    float x=0.7;
    float y=0;
    if(cotes > 0 && rayon > 0 && rayon_trou > 0)
    {
        glBegin(GL_TRIANGLE_STRIP);
        //-----sommets-----
        glNormal3f(0,0,1);
        for(int i = 0; i <= cotes; i++)
        {
            double x1 = cos(i*2*M_PI/cotes);
            double y1 = sin(i*2*M_PI/cotes);
            glTexCoord2f((1+(x1*rayon_trou+x)/rayon)/2, (1+(y1*rayon_trou+y)/rayon)/2);
            glVertex3f(rayon_trou*x1+x, rayon_trou*y1+y,0);
            glTexCoord2f((1+x1)/2, (1+y1)/2);
            glVertex3f(rayon*x1,rayon*y1,0);
        }
        glEnd();
    }
    //Fin de la liste pour un disque
    glEndList ();

    //Liste pour le cube
    glNewList(displaylistbras[1], GL_COMPILE);
    glBegin(GL_QUADS);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);
    glVertex3f(0,0,0);
    glTexCoord2f(0,1);
    glVertex3f(1,0,0);
    glTexCoord2f(1,1);
    glVertex3f(1,0,1);
    glTexCoord2f(1,0);
    glVertex3f(0,0,1);

    glNormal3f(1,0,0);
    glTexCoord2f(0,0);
    glVertex3f(1,0,0);
    glTexCoord2f(0,1);
    glVertex3f(1,1,0);
    glTexCoord2f(1,1);
    glVertex3f(1,1,1);
    glTexCoord2f(1,0);
    glVertex3f(1,0,1);

    glNormal3f(0,1,0);
    glTexCoord2f(0,1);
    glVertex3f(1,1,0);
    glTexCoord2f(0,0);
    glVertex3f(0,1,0);
    glTexCoord2f(1,0);
    glVertex3f(0,1,1);
    glTexCoord2f(1,1);
    glVertex3f(1,1,1);

    glNormal3f(-1,0,0);
    glTexCoord2f(0,1);
    glVertex3f(0,1,0);
    glTexCoord2f(0,0);
    glVertex3f(0,0,0);
    glTexCoord2f(1,0);
    glVertex3f(0,0,1);
    glTexCoord2f(1,1);
    glVertex3f(0,1,1);

    glNormal3f(0,0,-1);
    glVertex3f(0,0,0);
    glVertex3f(0,1,0);
    glVertex3f(1,1,0);
    glVertex3f(1,0,0);

    glNormal3f(0,0,1);
    glVertex3f(0,0,1);
    glVertex3f(0,1,1);
    glVertex3f(1,1,1);
    glVertex3f(1,0,1);
    glEnd ();
    //Fin liste du cube
    glEndList();



    //Liste
    glNewList(displaylistbras[2],GL_COMPILE);
    glBegin(GL_QUADS);

    //Face devant
    glVertex3f(1,0,0);
    glVertex3f(1,1,0);
    glVertex3f(1,1,1);
    glVertex3f(1,0,1);
    //Face fond
    glVertex3f(0,0,0);
    glVertex3f(0,0,1);
    glVertex3f(0,1,1);
    glVertex3f(0,1,0);
    //Face droite
    glVertex3f(0,0,0);
    glVertex3f(0,1,0);
    glVertex3f(1,1,0);
    glVertex3f(1,0,0);

    //Face gauche
    glVertex3f(0,0,1);
    glVertex3f(1,0,1);
    glVertex3f(1,1,1);
    glVertex3f(0,1,1);

    //Face dessous (sol)
    glVertex3f(0,0,0);
    glVertex3f(1,0,0);
    glVertex3f(1,0,1);
    glVertex3f(0,0,1);

    glColor4f(255,255,255,1);
    //Face dessus
    glVertex3f(0,1,0);
    glVertex3f(0,1,1);
    glVertex3f(1,1,1);
    glVertex3f(1,1,0);


    glEnd();
    glEndList ();

    //Liste pour la sphere
    glNewList(displaylistbras[3],GL_COMPILE);

    int divSphere1 = 200; //permet de gerer la resolution
    int divSphere2 = 200; //permet de gerer la resolution
    float floatSpherea = ( M_PI / divSphere1 );
    float floatSphereb = ( 2.0f * M_PI / divSphere2 );
    glBegin(GL_QUADS);
    glPushMatrix ();
    for( int i = 0; i < divSphere1 + 1 ; i++ )
    {
        float r0 = sin ( i * floatSpherea ); //si on ajoute un entier assez grand, la sphere s'applatit pour devenir un disque
        float y0 = cos ( i * floatSpherea ) ;
        float r1 = sin ( (i+1) * floatSpherea ) ;
        float y1 = cos ( (i+1) * floatSpherea ) ;

        for( int j = 0; j < divSphere2 + 1 ; j++ )
        {
            float x0 = r0 * sin( j * floatSphereb ) ; //si on ajoute un grand entier, la sphere s'allonge comme en forme de "gellule"
            float z0 = r0 * cos( j * floatSphereb ) ;
            float x1 = r0 * sin( (j+1) * floatSphereb ) ;
            float z1 = r0 * cos( (j+1) * floatSphereb ) ;
            float x2 = r1 * sin( j * floatSphereb ) ;
            float z2 = r1 * cos( j * floatSphereb ) ;
            float x3 = r1 * sin( (j+1) * floatSphereb ) ;
            float z3 = r1 * cos( (j+1) * floatSphereb ) ;
            int c1, c2, c3;

            c1 = j / divSphere2;
            c2 = (j*(j/divSphere2) + divSphere2) / divSphere2;
            c3 = (j + divSphere2) / divSphere2;

            if (j==0)
            {
                glTexCoord2f(0,0);
            }
            else
            {
                glTexCoord2f(c1,c2);
            }
            glVertex3f(x0,y0,z0);
            glTexCoord2f (c3,c2);
            glVertex3f(x1,y0,z1);
            glTexCoord2f (c2,c3);
            glVertex3f(x3,y1,z3);
            if (j==divSphere2)
            {
                glTexCoord2f(1,1);
            }
            else
            {
                glTexCoord2f(c2,c3);
            }
            glTexCoord2f (c2,c2);
            glVertex3f(x2,y1,z2);
        }
    }
    glPopMatrix();
    glEnd();

    //Fin de la liste pour la sphere
    glEndList ();

    glNewList(displaylistbras[4],GL_COMPILE);
    glBegin ( GL_TRIANGLES );
    glNormal3f(0.702714,-1.06018e-008,-0.711473);
    glVertex3f(0.5,0,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(0.66832,0.21715,-0.711473);
    glVertex3f(0.475528,0.154509,-0.5);
    glNormal3f(0.66832,0.21715,-0.711473);
    glVertex3f(0.475528,0.154509,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(0.568507,0.413045,-0.711473);
    glVertex3f(0.404509,0.293893,-0.5);
    glNormal3f(0.568507,0.413045,-0.711473);
    glVertex3f(0.404509,0.293893,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(0.413045,0.568507,-0.711473);
    glVertex3f(0.293893,0.404509,-0.5);
    glNormal3f(0.413045,0.568507,-0.711473);
    glVertex3f(0.293893,0.404509,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(0.21715,0.66832,-0.711473);
    glVertex3f(0.154508,0.475528,-0.5);
    glNormal3f(0.21715,0.66832,-0.711473);
    glVertex3f(0.154508,0.475528,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(-6.36106e-008,0.702713,-0.711473);
    glVertex3f(-2.18557e-008,0.5,-0.5);
    glNormal3f(-6.36106e-008,0.702713,-0.711473);
    glVertex3f(-2.18557e-008,0.5,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(-0.21715,0.66832,-0.711473);
    glVertex3f(-0.154509,0.475528,-0.5);
    glNormal3f(-0.21715,0.66832,-0.711473);
    glVertex3f(-0.154509,0.475528,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(-0.413045,0.568507,-0.711473);
    glVertex3f(-0.293893,0.404509,-0.5);
    glNormal3f(-0.413045,0.568507,-0.711473);
    glVertex3f(-0.293893,0.404509,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(-0.568507,0.413045,-0.711473);
    glVertex3f(-0.404509,0.293893,-0.5);
    glNormal3f(-0.568507,0.413045,-0.711473);
    glVertex3f(-0.404509,0.293893,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(-0.66832,0.21715,-0.711473);
    glVertex3f(-0.475528,0.154508,-0.5);
    glNormal3f(-0.66832,0.21715,-0.711473);
    glVertex3f(-0.475528,0.154508,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(-0.702714,-6.36106e-008,-0.711473);
    glVertex3f(-0.5,-4.37114e-008,-0.5);
    glNormal3f(-0.702714,-6.36106e-008,-0.711473);
    glVertex3f(-0.5,-4.37114e-008,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(-0.66832,-0.21715,-0.711473);
    glVertex3f(-0.475528,-0.154508,-0.5);
    glNormal3f(-0.66832,-0.21715,-0.711473);
    glVertex3f(-0.475528,-0.154508,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(-0.568507,-0.413045,-0.711473);
    glVertex3f(-0.404508,-0.293893,-0.5);
    glNormal3f(-0.568507,-0.413045,-0.711473);
    glVertex3f(-0.404508,-0.293893,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(-0.413045,-0.568507,-0.711473);
    glVertex3f(-0.293893,-0.404509,-0.5);
    glNormal3f(-0.413045,-0.568507,-0.711473);
    glVertex3f(-0.293893,-0.404509,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(-0.21715,-0.66832,-0.711473);
    glVertex3f(-0.154509,-0.475528,-0.5);
    glNormal3f(-0.21715,-0.66832,-0.711473);
    glVertex3f(-0.154509,-0.475528,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(0,-0.702714,-0.711473);
    glVertex3f(5.96244e-009,-0.5,-0.5);
    glNormal3f(0,-0.702714,-0.711473);
    glVertex3f(5.96244e-009,-0.5,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(0.217151,-0.66832,-0.711473);
    glVertex3f(0.154509,-0.475528,-0.5);
    glNormal3f(0.217151,-0.66832,-0.711473);
    glVertex3f(0.154509,-0.475528,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(0.413045,-0.568507,-0.711473);
    glVertex3f(0.293893,-0.404508,-0.5);
    glNormal3f(0.413045,-0.568507,-0.711473);
    glVertex3f(0.293893,-0.404508,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(0.568507,-0.413044,-0.711473);
    glVertex3f(0.404509,-0.293892,-0.5);
    glNormal3f(0.568507,-0.413044,-0.711473);
    glVertex3f(0.404509,-0.293892,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(0.66832,-0.21715,-0.711473);
    glVertex3f(0.475528,-0.154508,-0.5);
    glNormal3f(0.66832,-0.21715,-0.711473);
    glVertex3f(0.475528,-0.154508,-0.5);
    glNormal3f(0,0,-1);
    glVertex3f(0,0,-0.5);
    glNormal3f(0.702714,-1.06018e-008,-0.711473);
    glVertex3f(0.5,0,-0.5);
    glNormal3f(0.66832,0.21715,0.711473);
    glVertex3f(0.475528,0.154509,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(0.702714,-1.06018e-008,0.711473);
    glVertex3f(0.5,0,0.5);
    glNormal3f(0.568507,0.413045,0.711473);
    glVertex3f(0.404509,0.293893,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(0.66832,0.21715,0.711473);
    glVertex3f(0.475528,0.154509,0.5);
    glNormal3f(0.413045,0.568507,0.711473);
    glVertex3f(0.293893,0.404509,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(0.568507,0.413045,0.711473);
    glVertex3f(0.404509,0.293893,0.5);
    glNormal3f(0.21715,0.66832,0.711473);
    glVertex3f(0.154508,0.475528,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(0.413045,0.568507,0.711473);
    glVertex3f(0.293893,0.404509,0.5);
    glNormal3f(-6.36106e-008,0.702713,0.711473);
    glVertex3f(-2.18557e-008,0.5,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(0.21715,0.66832,0.711473);
    glVertex3f(0.154508,0.475528,0.5);
    glNormal3f(-0.21715,0.66832,0.711473);
    glVertex3f(-0.154509,0.475528,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(-6.36106e-008,0.702713,0.711473);
    glVertex3f(-2.18557e-008,0.5,0.5);
    glNormal3f(-0.413045,0.568507,0.711473);
    glVertex3f(-0.293893,0.404509,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(-0.21715,0.66832,0.711473);
    glVertex3f(-0.154509,0.475528,0.5);
    glNormal3f(-0.568507,0.413045,0.711473);
    glVertex3f(-0.404509,0.293893,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(-0.413045,0.568507,0.711473);
    glVertex3f(-0.293893,0.404509,0.5);
    glNormal3f(-0.66832,0.21715,0.711473);
    glVertex3f(-0.475528,0.154508,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(-0.568507,0.413045,0.711473);
    glVertex3f(-0.404509,0.293893,0.5);
    glNormal3f(-0.702714,-6.36106e-008,0.711473);
    glVertex3f(-0.5,-4.37114e-008,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(-0.66832,0.21715,0.711473);
    glVertex3f(-0.475528,0.154508,0.5);
    glNormal3f(-0.66832,-0.21715,0.711473);
    glVertex3f(-0.475528,-0.154508,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(-0.702714,-6.36106e-008,0.711473);
    glVertex3f(-0.5,-4.37114e-008,0.5);
    glNormal3f(-0.568507,-0.413045,0.711473);
    glVertex3f(-0.404508,-0.293893,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(-0.66832,-0.21715,0.711473);
    glVertex3f(-0.475528,-0.154508,0.5);
    glNormal3f(-0.413045,-0.568507,0.711473);
    glVertex3f(-0.293893,-0.404509,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(-0.568507,-0.413045,0.711473);
    glVertex3f(-0.404508,-0.293893,0.5);
    glNormal3f(-0.21715,-0.66832,0.711473);
    glVertex3f(-0.154509,-0.475528,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(-0.413045,-0.568507,0.711473);
    glVertex3f(-0.293893,-0.404509,0.5);
    glNormal3f(0,-0.702714,0.711473);
    glVertex3f(5.96244e-009,-0.5,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(-0.21715,-0.66832,0.711473);
    glVertex3f(-0.154509,-0.475528,0.5);
    glNormal3f(0.217151,-0.66832,0.711473);
    glVertex3f(0.154509,-0.475528,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(0,-0.702714,0.711473);
    glVertex3f(5.96244e-009,-0.5,0.5);
    glNormal3f(0.413045,-0.568507,0.711473);
    glVertex3f(0.293893,-0.404508,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(0.217151,-0.66832,0.711473);
    glVertex3f(0.154509,-0.475528,0.5);
    glNormal3f(0.568507,-0.413044,0.711473);
    glVertex3f(0.404509,-0.293892,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(0.413045,-0.568507,0.711473);
    glVertex3f(0.293893,-0.404508,0.5);
    glNormal3f(0.66832,-0.21715,0.711473);
    glVertex3f(0.475528,-0.154508,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(0.568507,-0.413044,0.711473);
    glVertex3f(0.404509,-0.293892,0.5);
    glNormal3f(0.702714,-1.06018e-008,0.711473);
    glVertex3f(0.5,0,0.5);
    glNormal3f(0,0,1);
    glVertex3f(0,0,0.5);
    glNormal3f(0.66832,-0.21715,0.711473);
    glVertex3f(0.475528,-0.154508,0.5);
    glEnd ();
    glBegin ( GL_QUADS );
    glNormal3f(0.702714,-1.06018e-008,-0.711473);
    glVertex3f(0.5,0,-0.5);
    glNormal3f(0.66832,0.21715,-0.711473);
    glVertex3f(0.475528,0.154509,-0.5);
    glNormal3f(0.66832,0.21715,0.711473);
    glVertex3f(0.475528,0.154509,0.5);
    glNormal3f(0.702714,-1.06018e-008,0.711473);
    glVertex3f(0.5,0,0.5);
    glNormal3f(0.66832,0.21715,-0.711473);
    glVertex3f(0.475528,0.154509,-0.5);
    glNormal3f(0.568507,0.413045,-0.711473);
    glVertex3f(0.404509,0.293893,-0.5);
    glNormal3f(0.568507,0.413045,0.711473);
    glVertex3f(0.404509,0.293893,0.5);
    glNormal3f(0.66832,0.21715,0.711473);
    glVertex3f(0.475528,0.154509,0.5);
    glNormal3f(0.568507,0.413045,-0.711473);
    glVertex3f(0.404509,0.293893,-0.5);
    glNormal3f(0.413045,0.568507,-0.711473);
    glVertex3f(0.293893,0.404509,-0.5);
    glNormal3f(0.413045,0.568507,0.711473);
    glVertex3f(0.293893,0.404509,0.5);
    glNormal3f(0.568507,0.413045,0.711473);
    glVertex3f(0.404509,0.293893,0.5);
    glNormal3f(0.413045,0.568507,-0.711473);
    glVertex3f(0.293893,0.404509,-0.5);
    glNormal3f(0.21715,0.66832,-0.711473);
    glVertex3f(0.154508,0.475528,-0.5);
    glNormal3f(0.21715,0.66832,0.711473);
    glVertex3f(0.154508,0.475528,0.5);
    glNormal3f(0.413045,0.568507,0.711473);
    glVertex3f(0.293893,0.404509,0.5);
    glNormal3f(0.21715,0.66832,-0.711473);
    glVertex3f(0.154508,0.475528,-0.5);
    glNormal3f(-6.36106e-008,0.702713,-0.711473);
    glVertex3f(-2.18557e-008,0.5,-0.5);
    glNormal3f(-6.36106e-008,0.702713,0.711473);
    glVertex3f(-2.18557e-008,0.5,0.5);
    glNormal3f(0.21715,0.66832,0.711473);
    glVertex3f(0.154508,0.475528,0.5);
    glNormal3f(-6.36106e-008,0.702713,-0.711473);
    glVertex3f(-2.18557e-008,0.5,-0.5);
    glNormal3f(-0.21715,0.66832,-0.711473);
    glVertex3f(-0.154509,0.475528,-0.5);
    glNormal3f(-0.21715,0.66832,0.711473);
    glVertex3f(-0.154509,0.475528,0.5);
    glNormal3f(-6.36106e-008,0.702713,0.711473);
    glVertex3f(-2.18557e-008,0.5,0.5);
    glNormal3f(-0.21715,0.66832,-0.711473);
    glVertex3f(-0.154509,0.475528,-0.5);
    glNormal3f(-0.413045,0.568507,-0.711473);
    glVertex3f(-0.293893,0.404509,-0.5);
    glNormal3f(-0.413045,0.568507,0.711473);
    glVertex3f(-0.293893,0.404509,0.5);
    glNormal3f(-0.21715,0.66832,0.711473);
    glVertex3f(-0.154509,0.475528,0.5);
    glNormal3f(-0.413045,0.568507,-0.711473);
    glVertex3f(-0.293893,0.404509,-0.5);
    glNormal3f(-0.568507,0.413045,-0.711473);
    glVertex3f(-0.404509,0.293893,-0.5);
    glNormal3f(-0.568507,0.413045,0.711473);
    glVertex3f(-0.404509,0.293893,0.5);
    glNormal3f(-0.413045,0.568507,0.711473);
    glVertex3f(-0.293893,0.404509,0.5);
    glNormal3f(-0.568507,0.413045,-0.711473);
    glVertex3f(-0.404509,0.293893,-0.5);
    glNormal3f(-0.66832,0.21715,-0.711473);
    glVertex3f(-0.475528,0.154508,-0.5);
    glNormal3f(-0.66832,0.21715,0.711473);
    glVertex3f(-0.475528,0.154508,0.5);
    glNormal3f(-0.568507,0.413045,0.711473);
    glVertex3f(-0.404509,0.293893,0.5);
    glNormal3f(-0.66832,0.21715,-0.711473);
    glVertex3f(-0.475528,0.154508,-0.5);
    glNormal3f(-0.702714,-6.36106e-008,-0.711473);
    glVertex3f(-0.5,-4.37114e-008,-0.5);
    glNormal3f(-0.702714,-6.36106e-008,0.711473);
    glVertex3f(-0.5,-4.37114e-008,0.5);
    glNormal3f(-0.66832,0.21715,0.711473);
    glVertex3f(-0.475528,0.154508,0.5);
    glNormal3f(-0.702714,-6.36106e-008,-0.711473);
    glVertex3f(-0.5,-4.37114e-008,-0.5);
    glNormal3f(-0.66832,-0.21715,-0.711473);
    glVertex3f(-0.475528,-0.154508,-0.5);
    glNormal3f(-0.66832,-0.21715,0.711473);
    glVertex3f(-0.475528,-0.154508,0.5);
    glNormal3f(-0.702714,-6.36106e-008,0.711473);
    glVertex3f(-0.5,-4.37114e-008,0.5);
    glNormal3f(-0.66832,-0.21715,-0.711473);
    glVertex3f(-0.475528,-0.154508,-0.5);
    glNormal3f(-0.568507,-0.413045,-0.711473);
    glVertex3f(-0.404508,-0.293893,-0.5);
    glNormal3f(-0.568507,-0.413045,0.711473);
    glVertex3f(-0.404508,-0.293893,0.5);
    glNormal3f(-0.66832,-0.21715,0.711473);
    glVertex3f(-0.475528,-0.154508,0.5);
    glNormal3f(-0.568507,-0.413045,-0.711473);
    glVertex3f(-0.404508,-0.293893,-0.5);
    glNormal3f(-0.413045,-0.568507,-0.711473);
    glVertex3f(-0.293893,-0.404509,-0.5);
    glNormal3f(-0.413045,-0.568507,0.711473);
    glVertex3f(-0.293893,-0.404509,0.5);
    glNormal3f(-0.568507,-0.413045,0.711473);
    glVertex3f(-0.404508,-0.293893,0.5);
    glNormal3f(-0.413045,-0.568507,-0.711473);
    glVertex3f(-0.293893,-0.404509,-0.5);
    glNormal3f(-0.21715,-0.66832,-0.711473);
    glVertex3f(-0.154509,-0.475528,-0.5);
    glNormal3f(-0.21715,-0.66832,0.711473);
    glVertex3f(-0.154509,-0.475528,0.5);
    glNormal3f(-0.413045,-0.568507,0.711473);
    glVertex3f(-0.293893,-0.404509,0.5);
    glNormal3f(-0.21715,-0.66832,-0.711473);
    glVertex3f(-0.154509,-0.475528,-0.5);
    glNormal3f(0,-0.702714,-0.711473);
    glVertex3f(5.96244e-009,-0.5,-0.5);
    glNormal3f(0,-0.702714,0.711473);
    glVertex3f(5.96244e-009,-0.5,0.5);
    glNormal3f(-0.21715,-0.66832,0.711473);
    glVertex3f(-0.154509,-0.475528,0.5);
    glNormal3f(0,-0.702714,-0.711473);
    glVertex3f(5.96244e-009,-0.5,-0.5);
    glNormal3f(0.217151,-0.66832,-0.711473);
    glVertex3f(0.154509,-0.475528,-0.5);
    glNormal3f(0.217151,-0.66832,0.711473);
    glVertex3f(0.154509,-0.475528,0.5);
    glNormal3f(0,-0.702714,0.711473);
    glVertex3f(5.96244e-009,-0.5,0.5);
    glNormal3f(0.217151,-0.66832,-0.711473);
    glVertex3f(0.154509,-0.475528,-0.5);
    glNormal3f(0.413045,-0.568507,-0.711473);
    glVertex3f(0.293893,-0.404508,-0.5);
    glNormal3f(0.413045,-0.568507,0.711473);
    glVertex3f(0.293893,-0.404508,0.5);
    glNormal3f(0.217151,-0.66832,0.711473);
    glVertex3f(0.154509,-0.475528,0.5);
    glNormal3f(0.413045,-0.568507,-0.711473);
    glVertex3f(0.293893,-0.404508,-0.5);
    glNormal3f(0.568507,-0.413044,-0.711473);
    glVertex3f(0.404509,-0.293892,-0.5);
    glNormal3f(0.568507,-0.413044,0.711473);
    glVertex3f(0.404509,-0.293892,0.5);
    glNormal3f(0.413045,-0.568507,0.711473);
    glVertex3f(0.293893,-0.404508,0.5);
    glNormal3f(0.568507,-0.413044,-0.711473);
    glVertex3f(0.404509,-0.293892,-0.5);
    glNormal3f(0.66832,-0.21715,-0.711473);
    glVertex3f(0.475528,-0.154508,-0.5);
    glNormal3f(0.66832,-0.21715,0.711473);
    glVertex3f(0.475528,-0.154508,0.5);
    glNormal3f(0.568507,-0.413044,0.711473);
    glVertex3f(0.404509,-0.293892,0.5);
    glNormal3f(0.66832,-0.21715,-0.711473);
    glVertex3f(0.475528,-0.154508,-0.5);
    glNormal3f(0.702714,-1.06018e-008,-0.711473);
    glVertex3f(0.5,0,-0.5);
    glNormal3f(0.702714,-1.06018e-008,0.711473);
    glVertex3f(0.5,0,0.5);
    glNormal3f(0.66832,-0.21715,0.711473);
    glVertex3f(0.475528,-0.154508,0.5);
    glEnd ();
    glEndList ();


    //Début de la liste pour un disque
    glNewList (displaylistbras[5], GL_COMPILE );
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5,0.5);
    glVertex3f(0,0,0);
    //-----sommets-----
    for(int i = 0; i < 361; i++)
    {
        float x12 = cos(i*M_PI/180);
        float y12 = sin(i*M_PI/180);
        if(i==0)
        {
            glTexCoord2f(1,0.5);
        }
        if(i==90)
        {
            glTexCoord2f(0.5,1);
        }
        if(i==180)
        {
            glTexCoord2f(0,0.5);
        }
        if(i==270)
        {
            glTexCoord2f(0.5,0);
        }
        if(i==360)
        {
            glTexCoord2f(1,0.5);
        }

        glVertex3f(rayon*x12,rayon*y12,0);
    }
    glEnd();
    //Fin de la liste pour un disque
    glEndList ();

}

Mat MyGLWidget::loadFromQrc(QString qrc, int flag)
{
    QFile file(qrc);
    Mat m;
    if(file.open(QIODevice::ReadOnly))
    {
        qint64 sz = file.size();
        std::vector<uchar> buf(sz);
        file.read((char*)buf.data(), sz);
        m = imdecode(buf, flag);
    }
    return m;
}

GLuint MyGLWidget::loadTexture(QString filepath)
{
    GLuint textureTrash;
    glEnable(GL_TEXTURE_2D);
    Mat image = loadFromQrc(filepath,IMREAD_COLOR);
    if(image.empty()){
        std::cout << "image empty" << std::endl;
    }else{
        cv::flip(image, image, 0);
        glGenTextures(1, &textureTrash);
        glBindTexture(GL_TEXTURE_2D, textureTrash);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Set texture clamping method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


        glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGB,            // Internal colour format to convert to
                     image.cols,          // Image width  i.e. 640 for Kinect in standard mode
                     image.rows,          // Image height i.e. 480 for Kinect in standard mode
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,  // Image data type
                     image.ptr());        // The actual image data itself

    }
    return textureTrash;
}

void MyGLWidget::drawCage()
{
    //  glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    // glEnable(GL_CULL_FACE);
    glEnable ( GL_NORMALIZE );
    glDepthMask ( GL_TRUE );
    glPointSize ( 1.0f );
    glLineWidth ( 1.0f );
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glBegin(GL_QUADS);

    //Face arr texture
    glColor3f(1,1,1);
    glTexCoord2f(1.0f, 0.0f);glVertex3f(0,0.8,0);
    glTexCoord2f(1.0f, 1.0f);glVertex3f(0,1,0);
    glTexCoord2f(0.0f, 1.0f);glVertex3f(1,1,0);
    glTexCoord2f(0.0f, 0.0f);glVertex3f(1,0.8,0);


    //Face droite texture
    glColor3f(1,1,1);
    glTexCoord2f(1.0f, 0.0f);glVertex3f(1,0.8,0);
    glTexCoord2f(1.0f, 1.0f);glVertex3f(1,1,0);
    glTexCoord2f(0.0f, 1.0f);glVertex3f(1,1,1);
    glTexCoord2f(0.0f, 0.0f);glVertex3f(1,0.8,1);


    //Face av texture
    glColor3f(1,1,1);
    glTexCoord2f(1.0f, 0.0f);glVertex3f(1,0.8,1);
    glTexCoord2f(1.0f, 1.0f);glVertex3f(1,1,1);
    glTexCoord2f(0.0f, 1.0f);glVertex3f(0,1,1);
    glTexCoord2f(0.0f, 0.0f);glVertex3f(0,0.8,1);


    //Face gauche texture
    glColor3f(1,1,1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0,0.8,1);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0,1,1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0,1,0);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0,0.8,0);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    //Face haut

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0,1,0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0,1,1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1,1,1);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1,1,0);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    glBegin(GL_QUADS);

    //Face bas
    glColor3f(1,1,1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0,0,0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0.25,0,0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0.25,0,1);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0,0,1);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0.25,0,0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0.5,0,0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0.5,0,1);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.25,0,1);


    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0.5,0,0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0.75,0,0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0.75,0,1);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.5,0,1);

    glColor3f(1,1,1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0.75,0,0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1,0,0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1,0,0.75);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.75,0,0.75);

    glEnd();

    // TRANSPARENCE
    glDisable(GL_TEXTURE_2D);
    glEnable( GL_BLEND );
    glDepthMask (GL_FALSE);
    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

    glBegin(GL_QUADS);
    //Face arr transparent
    glColor4f(0.9,0.9,0.9,0.8);
    glVertex3f(0,0,0);
    glVertex3f(0,0.8,0);
    glVertex3f(1,0.8,0);
    glVertex3f(1,0,0);

    //Face droite transparent
    glColor4f(0.9,0.9,0.9,0.8);
    glVertex3f(1,0,0);
    glVertex3f(1,0.8,0);
    glVertex3f(1,0.8,1);
    glVertex3f(1,0,1);

    //Face gauche transparent
    glColor4f(0.9,0.9,0.9,0.8);
    glVertex3f(0,0,1);
    glVertex3f(0,0.8,1);
    glVertex3f(0,0.8,0);
    glVertex3f(0,0,0);

    //Face av transparent
    glColor4f(0.9,0.9,0.9,0.8);
    glVertex3f(1,0,1);
    glVertex3f(1,0.8,1);
    glVertex3f(0,0.8,1);
    glVertex3f(0,0,1);

    glEnd();
    glPopMatrix();
    glDepthMask (GL_TRUE);
    glDisable( GL_BLEND );
    //glEnable( GL_CULL_FACE );
}

void MyGLWidget::draw_balle_dans_main(){

    glPushMatrix();
    glScalef(0.1,0.1,0.1);
    glScalef(taille_sphere,taille_sphere,taille_sphere);
    glCallList(GLdisplayList_[0]);
    //glutSolidSphere(0.5,50,50);

    glPopMatrix();

}

void MyGLWidget::draw_brasrobot(){

    //Le bras est rotate de 90°, les axes y et z sont inversés.
    //glDisable( GL_TEXTURE_2D );
    glPushMatrix();
    glScalef(0.17,0.17,0.17);
    glPushMatrix();
    glPushMatrix();
    glPushMatrix();

    //glDisable( GL_CULL_FACE );

    //Socle au plafond
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glPushMatrix();
    glColor3f(0.5,0.5,0.5);
    glTranslatef(-2,-2,0.9);
    glScalef(4,4,1);
    glCallList(displaylistbras[1]);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    //BIND TEXTURE
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glPushMatrix();
    glPushMatrix();
    //glColor3f(1,1,0);
    glTranslatef(-1,-1,1);
    glScalef(2,2,10);


    if(descente==true){                          //Descente du bras
        descendre_pince(0);
    }

    glCallList(displaylistbras[1]);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();


    glRotatef(3,0,0,1);
    glPushMatrix();

    //Sphere 3
    glPushMatrix();
    glColor3f(0.5,0.5,0.5);
    glTranslatef(0,0,12.5);
    glScalef(2,2,2);
    if(descente==true){
        descendre_pince(1);
    }

    glPushMatrix();
    glCallList(displaylistbras[3]);
    glPopMatrix();
    //PHALANGES
    glPushMatrix();
    glScalef(.4,0.4,0.4);
    glPushMatrix();
    //Doigt 1
    glPushMatrix();
    if(descente==true && remonte==false){
        glRotatef(30,0,1,0);
    }else{
        glRotatef(3,0,1,0);
    }

    glPushMatrix();
    //Phalange 1
    glPushMatrix();
    glColor3f(0.75,0.75,0.75);
    glTranslatef(1.5,0,2);
    glRotatef(20,0,1,0);
    glScalef(1,1,4);
    glCallList(displaylistbras[4]);
    glPopMatrix();  // fin phalange 1

    //Phalange 2
    glPushMatrix();
    glColor3f(0.75,0.75,0.75);
    glTranslatef(1.5,0,6);
    glRotatef(-20,0,1,0);
    glScalef(1,1,4);
    glCallList(displaylistbras[4]);
    glPopMatrix(); // fin phalange 2
    glPopMatrix();
    glPopMatrix(); // fin doigt 1

    //Doigt 2
    glPushMatrix();
    if(descente==true && remonte==false){
        glRotatef(-20,0,1,0);
    }else{
        glRotatef(1,0,1,0);
    }

    glPushMatrix();
    //Phalange 3
    glPushMatrix();
    glColor3f(0.75,0.75,0.75);
    glTranslatef(-1.5,0,2);
    glRotatef(-20,0,1,0);
    glScalef(1,1,4);
    glCallList(displaylistbras[4]);
    glPopMatrix(); //Fin phalange 3
    //Phalange 4
    glPushMatrix();
    glColor3f(0.75,0.75,0.75);
    glTranslatef(-1.5,0,6);
    glRotatef(20,0,1,0);
    glScalef(1,1,4);
    glCallList(displaylistbras[4]);
    glPopMatrix();// fin phalange 4
    glPopMatrix();
    glPopMatrix();//Fin doigt 2

    //Doigt 3
    glPushMatrix();
    if(descente==true && remonte==false){
        glRotatef(90,0,0,1);
    }else{
        glRotatef(45,0,0,1);
    }

    glRotatef(5,1,0,0);
    glTranslatef(0,-1,0);
    glPushMatrix();

    //Phalange 3
    glPushMatrix();
    glColor3f(0.75,0.75,0.75);
    glTranslatef(-1.5,0,2);
    glRotatef(-20,0,1,0);
    glScalef(1,1,4);
    glCallList(displaylistbras[4]);
    glPopMatrix(); //Fin phalange 3
    //Phalange 4
    glPushMatrix();
    glColor3f(0.75,0.75,0.75);
    glTranslatef(-1.5,0,6);
    glRotatef(20,0,1,0);
    glScalef(1,1,4);
    glCallList(displaylistbras[4]);
    glPopMatrix();// fin phalange 4
    glPopMatrix();
    glPopMatrix();//Fin doigt 3
    glPopMatrix();

    //Dessin de la balle dans la main robot quand on en a attrapé une
    glPushMatrix();


    mouvement_verticaux_brasrobot();

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

void MyGLWidget::animation_partie_gagnee(){

    //Cage en mode discotheque +affichage score réalisé(coté mainwindow)
    //setYRotation(yRot+5);
    //partie_lancee=false;

}

void MyGLWidget::drawTrou()
{
    //Reprise du modele de la cage qu'on a rétrécie et dont on a enlevé les textures
    glPushMatrix();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    // glEnable(GL_CULL_FACE);
    glEnable ( GL_NORMALIZE );
    glDepthMask ( GL_TRUE );
    glPointSize ( 1.0f );
    glLineWidth ( 1.0f );
    glEnable(GL_COLOR_MATERIAL);

    glTranslatef(-0.5,-0.5,-0.5);
    glPushMatrix();
    // glEnable(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, texture[0]);
    /* glBegin(GL_QUADS);
    //Face haut
    //glColor3f(0,0,1);
    glVertex3f(0,1,0);
    glVertex3f(0,1,1);
    glVertex3f(1,1,1);
    glVertex3f(1,1,0);
    glEnd();*/

    //glBindTexture(GL_TEXTURE_2D, texture[1]);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBegin(GL_QUADS);

    //Face bas

    glTexCoord2f(1.0f, 0.0f);glVertex3f(0,0,0);
    glTexCoord2f(1.0f, 1.0f);glVertex3f(1,0,0);
    glTexCoord2f(0.0f, 1.0f);glVertex3f(1,0,1);
    glTexCoord2f(0.0f, 0.0f);glVertex3f(0,0,1);

    glEnd();

    // TRANSPARENCE


    glBegin(GL_QUADS);
    //Face arr transparent
    glColor4f(0.9,0.9,0.9,0.8);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0,0,0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0,0.8,0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1,0.8,0);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1,0,0);

    //Face droite transparent
    glColor4f(0.9,0.9,0.9,0.8);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1,0,0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1,0.8,0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1,0.8,1);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1,0,1);

    //Face gauche transparent
    glColor4f(0.9,0.9,0.9,0.8);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0,0,1);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0,0.8,1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0,0.8,0);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0,0,0);

    glEnd();


    glDisable(GL_TEXTURE_2D);
    glEnable( GL_BLEND );
    glDepthMask (GL_FALSE);
    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
    glBegin(GL_QUADS);
    //Face av transparent
    glColor4f(0.9,0.9,0.9,0.8);
    glVertex3f(1,0,1);
    glVertex3f(1,0.8,1);
    glVertex3f(0,0.8,1);
    glVertex3f(0,0,1);

    glEnd();
    glPopMatrix();
    glDepthMask (GL_TRUE);
    glDisable( GL_BLEND );
    //glEnable( GL_CULL_FACE );
    glPopMatrix();
}



void MyGLWidget::draw()
{
    glPushMatrix();

    // Cage
    glPushMatrix();
    glTranslatef(-0.5,-0.5,-0.5);
    drawCage();
    glPopMatrix();

    glPushMatrix();

    //Placement des balles
    glPushMatrix();
    placement_spheres();
    glPopMatrix();

    // Trou pour déposer la balle
    glPushMatrix();
    glTranslatef(0.375,-0.575,0.375);
    glScalef(.25,.25,.25);
    drawTrou();
    glPopMatrix();
    //Bras robot
    glPushMatrix();
    //On centre le bras
    glTranslatef(0,-0.5,0);

    mouvement_horizontaux_brasrobot();

    glColor3f(0,0,0);
    glRotatef(90,1,0,0);
    glScalef(0.2,0.2,0.2);
    draw_brasrobot();
    glPopMatrix();
    glPopMatrix();

    glPopMatrix();

}
