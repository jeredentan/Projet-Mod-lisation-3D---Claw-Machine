#ifndef GAME_H
#define GAME_H

#include <QTime>
#include <QTimer>
#include <QWidget>
class game: public QObject
{
    Q_OBJECT
public:
    explicit game(QObject *parent = 0);



    int *scores_stockes;
    int score;
    int nouveau_top_normal;
    int nouveau_top_difficile;
    void update_score(int time);
    void comparer_top_3(int score,int difficulte);
};

#endif // GAME_H
