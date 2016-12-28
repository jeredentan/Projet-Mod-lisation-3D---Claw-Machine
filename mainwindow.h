#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDesktopWidget>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "myglwidget.h"
#include <cstdio>
#include <iostream>
#include "game.h"
using namespace cv;
using namespace std;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //timer d'update des affichages (webcam et mouvements bras robot).
    QTimer *tmrTimer;
    QTimer *timer_calcul_position;
    //timer d'update du temps écoulé et du compte à rebours
    QTimer *timer_update_temps;

    //Défini la difficulté du mode
    int difficulte;

    //Attributs relatifs au temps limite de partie
    bool message_temps_depasse_affiche;     //Permet de savoir quand afficher le popup pour le temps écoulé
    int temps_limite;               //temps maximal de la partie
    int compteur_temps_limite;      //compte à rebours que l'on décrémente dans le timer

    QMessageBox message_victoire;           //popup de victoire

    int temps_victoire;                 //temps mis par le joueur pour gagner
    int temps_sphere_courante;          //temps écouler depuis l'apparition de la dernière sphere
    int temps_total;                    //temps  écoulé depuis le debut de la partie

    game *partie;                   //instanciation de la classe game pour gérer le score du joueur

public slots:
    void updateaffichage();         //update de la webcam (slot lié à un timer)
    void updateposition();          //update des valeurs de la position du bras (lié à un timer)
    void lancer_partie();           //Permet d'initialiser une partie en fonciton du mode difficulté choisie
    void update_temps();            //timer pour l'update et l'affichage des différents temps


private:

    Ui::MainWindow *ui;
    //Attributs relatifs à la webcam et au calcul des vecteurs déplacement (flux optique)
    VideoCapture webcam;
    Mat frame;
    Rect rect;
    Mat frame1,frame2;
    Point p1,p2;
    Mat matrice_next_image;
    Mat flow;
    float um,vm;
    int frameWidth;
    int frameHeight;

    //Precision de la pince pour attraper une boule
    double precision;

    void keyPressEvent(QKeyEvent *ev);      //Fonction appelée quand barre espace enfoncée
    void generer_sphere();                  //Génère des position aléatoires pour la sphere courante
    void tout_reinitialiser();              //Réinitialise toutes les valeurs pour recommencer une aprtie
    bool isFlowCorrect(float u);
    void calc_mean(Mat flow1,float &um, float &vm);
};

#endif // MAINWINDOW_H
