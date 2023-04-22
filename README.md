# particules_simu
Rapide Description : Simulation de particules avec utilisation des "compute shaders" + effet de "blooming"

Description : Ce projet a été réalisé en tant que première prise en main des "computes shaders". L'idée étant de pouvoir simuler les déplacement d'un grand nombre de particules de manières indépendantes et en GPU. Le projet, dans son état actuel, positionne aléatoirement 70 000 particules dans un cube avec des vitesses aléatoires. Lorsque les particules s'éloignent trop du centre du cube elle replacées à la coordonnée de ce dernier pour en jaillir. Pour rendre ces particules le geometry shader dessine des disques toujours face à la caméra (les vertices sont multipliés par l'inverse de la matrice view).

Par la suite, j'ai souhaité rendre le résultat plus esthétique en ajoutant un effet de blooming.
Ce dernier se déroule ainsi :
  - Double rendu (rendu 1 classique de la scène, rendu 2 ne contenant que les zones à fortes luminance).
  - Constitution des mipmaps du rendu 2. Ces dernières permettent l'obtention rapide d'un plus large flou.
  - Gaussian blur two pass à partir des mipmaps du rendu 2
  - Combinaison du résultat obtenu avec le rendu 1

Je souhaite, pour finir ce mini-projet, se faire mouvoir les particules selon le gradient local d'un bruit de Perlin afin d'obtenir un résultat de "curling noise".

Lien youtube d'une capture vidéo du programme : LIEN YT

Ci-dessous une capture d'écran de l'éxecution du programme :
![Screenshot Particules](./screenshot_particules.png?raw=true)



