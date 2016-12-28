#include "game.h"
#include <QTimer>
#include <time.h>
#include <iostream>
#include <string>
#include <fstream>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <math.h>
#include <QDebug>
using namespace std;
game::game(QObject *parent) : QObject(parent)
{
    nouveau_top_difficile=-1;
    nouveau_top_normal=-1;
    score=1;
    scores_stockes= new int[6];
}

void game::update_score(int time){

    if(score>0){
        score = 1000/time +200;
    }


}

void game::comparer_top_3(int score,int difficulte){

    QFile file("/Users/Jeredentan/Documents/QtCreator/ProjetMultimedia/top3.txt");
    if(!file.open(QIODevice::ReadWrite)) {
        QMessageBox::information(0, "error", file.errorString());
    }

    QTextStream in(&file);

    int cpt=0;
    int k=0;
    while(!in.atEnd()) {
        QString line = in.readLine();

        scores_stockes[k]=line.toInt();
        cpt=cpt+1;
        k=k+1;
    }
    //On vide la fichier
    file.resize(0);
    //score stockes contient le contenu du fichier

    if(difficulte==0){
        if(scores_stockes[0]<score){
            scores_stockes[2]=scores_stockes[1];
            scores_stockes[1]=scores_stockes[0];
            scores_stockes[0]=score;
            nouveau_top_normal=score;
        }else{
            if(scores_stockes[1]<score){
                scores_stockes[2]=scores_stockes[1];
                scores_stockes[1]=score;
                nouveau_top_normal=score;
            }else{
                if(scores_stockes[2]<score){

                    scores_stockes[2]=score;
                    nouveau_top_normal=score;
                }
            }
        }

        if(nouveau_top_normal!=-1){
                QMessageBox msgbox;
                msgbox.setText("Vous êtes dans le top 3 des meilleurs scores en difficulté normale !!!  Brillant!!\n Top 3 : \n 1 - "+QString::number(scores_stockes[0])+"\n 2 - "+QString::number(scores_stockes[1])+"\n 3 - "+QString::number(scores_stockes[2]));
                msgbox.exec();
        }
    }else{
        if(difficulte==1){
            if(scores_stockes[3]<score){
                scores_stockes[5]=scores_stockes[4];
                scores_stockes[4]=scores_stockes[3];
                scores_stockes[0]=score;
                nouveau_top_difficile=score;
            }else{
                if(scores_stockes[4]<score){
                    scores_stockes[5]=scores_stockes[4];
                    scores_stockes[4]=score;
                    nouveau_top_difficile=score;
                }else{
                    if(scores_stockes[5]<score){
                        nouveau_top_difficile=score;
                        scores_stockes[5]=score;
                    }
                }
            }

            if(nouveau_top_difficile!=-1){
                QMessageBox msgbox;
                msgbox.setText("Vous êtes dans le top 3 des meilleurs scores en difficultée difficile !!! Brillant!!\n Top 3 : \n 1 - "+QString::number(scores_stockes[3])+"\n 2 - "+QString::number(scores_stockes[4])+"\n 3 - "+QString::number(scores_stockes[5]));
                msgbox.exec();
            }

        }
    }





    for(int i=0;i<6;i++){
        in << scores_stockes[i]<<endl;
    }
    // in << nouveau_score_top1<<endl;
    // in << nouveau_score_top2<<endl;
    // in << nouveau_score_top3<<endl;

    file.close();


    nouveau_top_difficile=-1;
    nouveau_top_normal=-1;
}


