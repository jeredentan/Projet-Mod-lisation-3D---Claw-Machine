
#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H
#include <QtWidgets>
#include <QtOpenGL>
#include <QGLWidget>
#include "glu.h"
#include "gl.h"
#include "opencv2/opencv.hpp"
#include <time.h>
#include <vector>
#include <QString>
#include <QGLWidget>
#include "glu.h"

using namespace std;
using namespace cv;

class MyGLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
    int xRot;
    int yRot;
    int zRot;

    double xPos;
    double yPos;
    double zPos;
    double xsphere;
    double ysphere;
    double zsphere;

    void placement_spheres();
    int nb_spheres_a_generer;
    double taille_sphere;

    int sphere_actuelle;
    double posX_sphereactuelle;
    double posZ_sphereactuelle;
    bool  placement_initial;
    double *tableau_positions_x;
    double *tableau_positions_z;

    bool partie_lancee;
    double y;
    bool descente;
    bool remonte;
    bool sphere_catched;
    bool deplacement_trou;
    bool lacher_balle;

    bool partie_gagnee;
    bool update_sphere_done;
    int precision_gl;
    double position_x_balle_attrapee;
    double position_z_balle_attrapee;

    void descendre_pince(int a);
    void remonter_pince(int a);
    void mouvement_horizontaux_brasrobot();
    void mouvement_verticaux_brasrobot();
    //void lacher_balle();
    bool repositionnement_temps_depasse;
    bool pince_reinitialisee;


protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);

    void setXTranslation(double pos);
    void setYTranslation(double pos);
    void setZTranslation(double pos);
        void animation_partie_gagnee();


signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

    void xTranslationChanged(double pos);
    void yTranslationChanged(double pos);
    void zTranslationChanged(double pos);


private:
       Mat loadFromQrc(QString qrc, int flag);


    void draw();
    void GLdisplayList();
    void drawCage();
    void drawTrou();
    void draw_balle_dans_main();
    void draw_brasrobot();
    void createdisplaylists();

    void balle_dans_bras_robot();
    void ramener_balle();
    void update_spheres_au_sol();

    GLuint loadTexture(QString filepath);
    GLuint GLdisplayList_[5];
    GLuint displaylistbras[5];
    GLuint texture[10];
    QPoint lastPos;

    QKeyEvent *keypressed;


     QTimer *timer_animation_finale;




    //int angle_=0;

};

#endif // MYGLWIDGET_H
