
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <QTimer>
#include <cstdio>
#include <math.h>
#include <iostream>
#include "myglwidget.h"
#include <game.h>
using namespace cv;
using namespace std;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->bouton_lancer,SIGNAL(clicked(bool)),this,SLOT(lancer_partie()));
    precision=3;

    ui->label_score->setText("1000");

    frameWidth=MainWindow::width()/2;
    frameHeight=MainWindow::height()/2;
    int subImageWidth=40;
    int subImageHeight=40;
    rect.x=(frameWidth-subImageWidth)/2;
    rect.y=frameHeight/2+(frameHeight/2-subImageHeight)/2;
    rect.width=subImageWidth;
    rect.height=subImageHeight;

    p1.x=rect.x+subImageWidth/2;
    p1.y=rect.y+subImageHeight/2;

    webcam.open(0);
    webcam.set(CV_CAP_PROP_FRAME_WIDTH,frameWidth);
    webcam.set(CV_CAP_PROP_FRAME_HEIGHT,frameHeight);
    if(webcam.isOpened()==false){
        return;
    }
    webcam.read(frame);
    cv::flip(frame,frame,1);
    Mat(frame,rect).copyTo(frame1);
    tmrTimer =new QTimer(this);
    connect(tmrTimer,SIGNAL(timeout()),this,SLOT(updateaffichage()));
    tmrTimer->start(20);
    timer_calcul_position=new QTimer(this);
    connect(timer_calcul_position,SIGNAL(timeout()),this,SLOT(updateposition()));
    timer_update_temps=new QTimer(this);
    connect(timer_update_temps,SIGNAL(timeout()),this,SLOT(update_temps()));
    ui->label_time->setText("0");
    ui->label_temps_sphere_courante->setText("0");
    ui->label_nb_spheres_restantes->setText("0");
    ui->label_temps_restant->setText("0");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::updateaffichage(){

    webcam.read(matrice_next_image);
    if(matrice_next_image.empty()==true) return;
    cv::cvtColor(matrice_next_image,matrice_next_image,CV_BGR2RGB);
    cv::flip(matrice_next_image,matrice_next_image,1);
    Mat(matrice_next_image,rect).copyTo(frame2);

    calcOpticalFlowSF(frame1, frame2,flow,3, 2, 4, 4.1, 25.5, 18, 55.0, 25.5, 0.35, 18, 55.0, 25.5, 10);
    calc_mean(flow,um,vm);
    rectangle(matrice_next_image,rect,Scalar( 0, 255, 0),2);

    p2.x=p1.x+4*um;
    p2.y=p1.y+4*vm;

    QImage qimgoriginal((uchar*)matrice_next_image.data,matrice_next_image.cols,matrice_next_image.rows,matrice_next_image.step,QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(qimgoriginal));

    swap(frame1,frame2);
}

bool MainWindow::isFlowCorrect(float u) {
    return !cvIsNaN(u) && (fabs(u) < 1e9);
}

void MainWindow::calc_mean(Mat flow1,float &um, float &vm) {
    float sumu = 0;
    float sumv = 0;
    int counter = 0;
    const int rows = flow1.rows;
    const int cols = flow1.cols;

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            Vec2f flow1_at_point = flow1.at<Vec2f>(y, x);
            float u1 = flow1_at_point[0];
            float v1 = flow1_at_point[1];

            if (isFlowCorrect(u1) && isFlowCorrect(v1)) {
                sumu += u1;
                sumv += v1;
                counter++;
            }
        }
    }
    um=sumu/(1e-9 + counter);
    vm=sumv/(1e-9 + counter);
}


void MainWindow::update_temps(){
    temps_total=ui->label_time->text().toInt();
    temps_total++;

    ui->label_time->setText(QString::number(temps_total));
    partie->update_score(temps_total);
    ui->label_score->setText(QString::number(partie->score));

    ui->label_temps_restant->setText(QString::number(compteur_temps_limite));
    compteur_temps_limite--;

    //Si on a attrapé toutes les spheres
    if(ui->GLWidget->sphere_actuelle==ui->GLWidget->nb_spheres_a_generer ){
        timer_update_temps->stop();
        temps_victoire=temps_total;
        partie->update_score(temps_total);

        //On compare le score au top 3 et on l'enregistre si il est supérieur
        partie->comparer_top_3(partie->score,difficulte);
    }

    //Si on n'a pas attrapé de balle, on incrémente le compteur pour la sphere courante
    if(ui->GLWidget->lacher_balle==false){
        temps_sphere_courante=ui->label_temps_sphere_courante->text().toInt();
        temps_sphere_courante++;
        ui->label_temps_sphere_courante->setText(QString::number(temps_sphere_courante));

    }else{
        //Si on a attrapé la balle est qu'on la dépose dans le trou, on réinitialise le timer pour la sphere courante
        if(ui->GLWidget->lacher_balle==true){
            temps_sphere_courante=0;
            ui->label_temps_sphere_courante->setText(QString::number(temps_sphere_courante));
        }
    }
}


void MainWindow::updateposition(){

    //On update la position (déplacement avec mouvements) seulement si la partie est lancée
    if(ui->GLWidget->partie_lancee==true){
        //Tant que la pince ne tente pas de récupérer une balle, on update la position de la pince avec les mouvements de la webcam
        if(fabs(um)>0.1 && fabs(vm)>0.1 && ui->GLWidget->descente==false && ui->GLWidget->deplacement_trou==false && ui->GLWidget->lacher_balle==false){
            ui->GLWidget->setXTranslation( um);
            ui->GLWidget->setZTranslation( vm);
        }else{
            //Quand la pince descent, on arrete d'update les positions x et z (z car la pince est rotate de 90°) et on fait varier yPos
            if(ui->GLWidget->descente==true && ui->GLWidget->deplacement_trou==false){

                if(ui->GLWidget->yPos<=-1.3 || ui->GLWidget->remonte==true){
                    ui->GLWidget->setYTranslation(2);               //En montée yPos varie positivement
                }else{
                    ui->GLWidget->setYTranslation(-2);             //En descente yPos varie négativement
                }
            }else{
                //La pince se déplace vers le trou, on refait varier xPos et zPos
                if(ui->GLWidget->deplacement_trou==true && ui->GLWidget->lacher_balle==false){
                    ui->GLWidget->setXTranslation(2);
                    ui->GLWidget->setZTranslation(2);
                }else{
                    //On descent la pince quand on est au dessus du trou
                    if(ui->GLWidget->lacher_balle==true){
                        ui->GLWidget->setYTranslation(-2);
                    }
                }
            }
        }


        //Cas ou on est en partie mais le temps limite est atteint
        if(temps_total>=temps_limite && message_temps_depasse_affiche==false){
            ui->GLWidget->repositionnement_temps_depasse=true;
            ui->GLWidget->deplacement_trou=true;
            QMessageBox msgBox;
            msgBox.setText("Temps dépassé");
            msgBox.exec();
            message_temps_depasse_affiche=true;
        }
        if(ui->GLWidget->pince_reinitialisee==true){
  //La limite de temps a été dépassée et la pince a été réinitialisée(replacée au dessus du trou) donc on remet tout à 0
            tout_reinitialiser();
        }
    }else{

        //Boule attrapée, on remet à 0 en generant une nouvelle boule
        if(ui->GLWidget->partie_gagnee==true){
            ui->GLWidget->sphere_actuelle=ui->GLWidget->sphere_actuelle+1;
            if(ui->GLWidget->sphere_actuelle==ui->GLWidget->nb_spheres_a_generer){

                QMessageBox msgBox1;
                QString temps=ui->label_time->text();
                QString scorefinal=QString::number(partie->score);
                msgBox1.setText("Partie terminée en "+temps+" secondes \n Votre score est de "+scorefinal);
                msgBox1.exec();
                ui->GLWidget->animation_partie_gagnee();
                ui->GLWidget->partie_lancee=false;
                ui->GLWidget->descente=false;
                ui->GLWidget->remonte=false;
                ui->GLWidget->deplacement_trou=false;
                ui->GLWidget->lacher_balle=false;
                ui->GLWidget->partie_gagnee=false;


                ui->bouton_lancer->setText("Lancer Partie");
                connect(ui->bouton_lancer,SIGNAL(clicked(bool)),this,SLOT(lancer_partie()));
            }else{
                generer_sphere();
                //On reset l'ensemble des boolean au cas ou
                ui->GLWidget->partie_lancee=true;
                ui->GLWidget->descente=false;
                ui->GLWidget->remonte=false;
                ui->GLWidget->deplacement_trou=false;
                ui->GLWidget->lacher_balle=false;
                ui->GLWidget->partie_gagnee=false;

            }

        }
    }


}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Space)
    {
        if(ui->GLWidget->partie_lancee==true && ui->GLWidget->partie_gagnee==false){
            ui->GLWidget->descente=true;

            double xpos=ui->GLWidget->xPos;
            double zpos=ui->GLWidget->zPos;
            double pos_sphere_x=ui->GLWidget->posX_sphereactuelle;
            double pos_sphere_z=ui->GLWidget->posZ_sphereactuelle;

            double distance=sqrt(pow((xpos-pos_sphere_x),2)+pow((zpos-pos_sphere_z),2));
            std::cout<<"distance : "<<distance<<std::endl;

            //On recupere la plus petite des distances
            if(distance<precision){

                ui->GLWidget->position_x_balle_attrapee=pos_sphere_x;
                ui->GLWidget->position_z_balle_attrapee=pos_sphere_z;
                ui->GLWidget->sphere_catched=true;

            }

        }
    }
    else
    {
        QMainWindow::keyPressEvent(ev);
    }
}
void MainWindow::lancer_partie(){



    //Conditions surs les modes de difficultés
    QMessageBox msgBox;
    msgBox.setText("Choisir la difficulté");
    QPushButton *buttonN = msgBox.addButton(tr("Normal"), QMessageBox::ActionRole);
    QPushButton *buttonD = msgBox.addButton(tr("Difficile"),QMessageBox::ActionRole);
    difficulte=msgBox.exec();


    if(difficulte==0){               //Difficulte normal
        tout_reinitialiser();

        partie=new game();

        ui->GLWidget->nb_spheres_a_generer=4;
        temps_limite=120;

        precision=0.15;         //Precision assez bonne

        compteur_temps_limite=temps_limite;
        ui->label_nb_spheres_restantes->setText(QString::number(ui->GLWidget->nb_spheres_a_generer));
        ui->GLWidget->taille_sphere=1.2;
        ui->GLWidget->partie_lancee=true;
        generer_sphere();


        timer_calcul_position->start(20);
        ui->bouton_lancer->setText("Partie en cours");
        timer_update_temps->start(1000);

    }else{
        if(difficulte==1){                 //Difficulte difficile
            tout_reinitialiser();

            temps_limite=60;
            precision=0.08;             //Precision assez petite pour rendre la chose pas simple

            compteur_temps_limite=temps_limite;
            partie=new game();


            ui->GLWidget->nb_spheres_a_generer=5;
            ui->label_nb_spheres_restantes->setText(QString::number(ui->GLWidget->nb_spheres_a_generer));
            ui->GLWidget->taille_sphere=0.9;

            generer_sphere();
            timer_calcul_position->start(20);
            ui->GLWidget->partie_lancee=true;
            ui->bouton_lancer->setText("Partie en cours");
            timer_update_temps->start(1000);
        }
    }

    ui->bouton_lancer->disconnect(ui->bouton_lancer,SIGNAL(clicked(bool)),this,SLOT(lancer_partie()));
}


void MainWindow::tout_reinitialiser(){
    ui->GLWidget->partie_lancee=false;
    ui->GLWidget->remonte=false;
    ui->GLWidget->descente=false;
    ui->GLWidget->partie_gagnee=false;
    ui->GLWidget->deplacement_trou=false;
    ui->GLWidget->lacher_balle=false;
    ui->GLWidget->repositionnement_temps_depasse=false;
    ui->GLWidget->sphere_catched=false;
    ui->GLWidget->update_sphere_done=false;
    timer_update_temps->stop();
    timer_calcul_position->stop();
    ui->label_time->setText("0");
    ui->label_temps_sphere_courante->setText("0");
    ui->label_nb_spheres_restantes->setText("0");
    ui->GLWidget->sphere_actuelle=0;
    ui->GLWidget->nb_spheres_a_generer=0;
    temps_sphere_courante=0;
    temps_total=0;
    temps_victoire=0;
    srand(time(NULL));
    message_temps_depasse_affiche=false;
    ui->GLWidget->pince_reinitialisee=false;
    ui->bouton_lancer->setText("Lancer Partie");
    connect(ui->bouton_lancer,SIGNAL(clicked(bool)),this,SLOT(lancer_partie()));

}

void MainWindow::generer_sphere(){
   //On genere une position aléatoire pour la boule
    double x= (rand()%70 +(-35));
    double z=(rand()%70 +(-35));
    x=x/100;
    z=z/100;
    ui->GLWidget->posX_sphereactuelle=x;
    ui->GLWidget->posZ_sphereactuelle=z;

}
